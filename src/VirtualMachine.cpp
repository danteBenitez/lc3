#define VM_DEBUG

#include <MemoryMappedRegister.h>
#include <Platform.h>

#include <Trap.h>
#include <Utils.h>
#include <VirtualMachine.h>
#include <iostream>

VirtualMachine::VirtualMachine() {
  set_condition_flag(ConditionFlag::ZRO);
  set_register(Register::PC, PC_START);
}

#ifdef VM_DEBUG
#define dbg(VALUE) std::cout << VALUE
#else
#define dbg(VALUE)
#endif

VirtualMachine::~VirtualMachine() = default;

void VirtualMachine::execute() {
  bool running = true;
  while (running) {
    // 1. Load one instruction from memory at the address of the PC
    // register.
    auto instruction = current_instruction();
    auto incremented_pc = get_register(Register::PC) + 1;

    if (incremented_pc >= VirtualMachine::MEMORY_MAX) {
      break;
    }

    // 2. Increment the PC register.
    set_register(Register::PC, incremented_pc);

    // 3. Look at the opcode to determine which type of
    // instruction it should perform.
    // 4. Perform the instruction using the parameters in the
    // instruction.
    if (perform(instruction) == ShouldBreak::Yes) {
      running = false;
    }

    // 5. Go back to step 1.
  }
}

VirtualMachine::ShouldBreak VirtualMachine::perform(Instruction instruction) {
  auto opcode = instruction.opcode();
  dbg((const void *)(get_register(Register::PC) - 1)
      << " Opcode: " << to_underlying(opcode) << "\n");
  switch (opcode) {
  case OpCode::ADD: {
    dbg("ADD instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 0x07);

    Register source_register1 =
        register_from_underlying((instruction.data() >> 6) & 0007);

    dbg("   Destination: " << register_name(destination_register) << "\n");
    dbg("   Source 1: " << register_name(source_register1) << "\n");

    // If bit [5] is 0, the second source operand is obtained from SR2. If bit
    // [5] is 1, the second source operand is obtained by sign -
    // extending the imm5 field to 16 bits.
    auto bit = (instruction.data() >> 5) & 0x1;
    dbg("   bit[5]: " << bit << "\n");

    auto operand1 = get_register(source_register1);
    auto operand2 = 0;

    if (bit == 0) {
      "   Register mode: \n";
      Register source_register2 =
          register_from_underlying((instruction.data()) & 07);
      dbg("     Source 2: " << register_name(source_register2) << "\n");
      operand2 = get_register(source_register2);
    } else {
      dbg("   Immediate mode: \n");
      auto imm5 = sign_extend(instruction.data() & 0x1f, 5);
      dbg("     Source 2: " << imm5 << "\n");
      operand2 = imm5;
    }

    uint16_t result = operand1 + operand2;
    dbg("   Result: " << result << "\n");

    // In both cases,
    // the second source operand is added to the contents of SR1 and the
    // result stored in DR. The condition codes are set, based on whether
    // the result is negative, zero, or positive.
    set_register(destination_register, result);

    break;
  }
  case OpCode::AND: {
    dbg("AND instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 0x07);

    Register source_register1 =
        register_from_underlying((instruction.data() >> 6) & 0x07);

    dbg("   Destination: " << register_name(destination_register) << "\n");
    dbg("   Source 1: " << register_name(source_register1) << "\n");

    auto bit = (instruction.data() >> 5) & 0x1;
    dbg("   bit[5]: " << bit << "\n");

    auto operand1 = get_register(source_register1);
    auto operand2 = 0;

    // If bit [5] is 0, the second source operand is obtained from SR2.
    if (bit == 0) {
      dbg("   Register mode: \n");
      Register source_register2 =
          register_from_underlying((instruction.data()) & 07);
      dbg("     Source 2: " << register_name(source_register2) << "\n");
      operand2 = get_register(source_register2);
    } else {
      // If bit [5] is 1, the second source operand is obtained by
      // sign-extending the imm5 field to 16 bits.
      dbg("   Immediate mode: \n");
      auto imm5 = sign_extend(instruction.data() & 0x1f, 5);
      dbg("   Source 2: " << imm5 << "\n");
      operand2 = imm5;
    }

    // In either case, the second source operand and
    // the contents of SR1 are bit- wise ANDed, and the result stored in DR.The
    // condition codes are set, based on whether the binary value produced,
    // taken as a 2’s complement integer, is negative, zero,
    // or positive
    auto result = operand1 & operand2;
    set_register(destination_register, result);

    break;
  }
  case OpCode::BR: {
    dbg("BR Instruction\n");
    // The condition codes specified by the state of bits [11:9] are tested.
    /// We could skip masking here since OpCode::BR === 0
    auto condition_codes = (instruction.data() >> 9) & 07;

    dbg("   Condition codes: " << (const void *)condition_codes << "\n");

    auto condition_flags = get_register(Register::COND);

    dbg("   Condition flags: " << (const void *)condition_flags << "\n");

    // If bit [11] is set, N is tested; if bit [11] is clear, N is not
    // tested. If bit [10] is set, Z is tested, etc. If any of the condition
    // codes tested is set, the program branches to the location specified
    // by adding the sign-extended PCoffset9 field to the incremented PC
    if (condition_codes & condition_flags) {
      auto pc_offset_9 = instruction.data() & 0x1ff;
      auto extended = sign_extend(pc_offset_9, 9);
      dbg("   Branching\n");
      auto incremented_pc = get_register(Register::PC);
      dbg("   to " << incremented_pc + extended);

      set_register(Register::PC, incremented_pc + extended,
                   ShouldUpdateCondition::No);
    }

    break;
  }
  case OpCode::JMP: {
    dbg("JMP Instruction\n");
    // The RET instruction is a special case of the JMP instruction. The PC
    // is loaded with the contents of R7, which contains the linkage back to
    // the instruction following the subroutine call instruction.

    Register base_register =
        register_from_underlying((instruction.data() >> 6) & 0x7);

    // The program unconditionally jumps to the location specified by the
    // contents of the base register. Bits [8:6] identify the base register.
    auto location = get_register(base_register);
    dbg("   Jumping to: " << (const void *)location << "\n");

    set_register(Register::PC, location, ShouldUpdateCondition::No);

    break;
  }
  case OpCode::JSR: {
    dbg("JSR instruction\n");

    // First, the incremented PC is saved in R7.
    // This is the linkage back to the calling
    // routine.
    set_register(Register::R7, get_register(Register::PC),
                 ShouldUpdateCondition::No);

    auto bit = (instruction.data() >> 11) & 0b1;
    auto address = 0;

    // The address of the subroutine is obtained from the base register (if bit
    // [11] is 0), or the address is computed by sign-extending bits [10:0] and
    // adding this value to the incremented PC (if bit [11] is 1)
    if (bit == 0) {
      Register base_register =
          register_from_underlying((instruction.data() >> 6) & 0x7);
      address = get_register(base_register);
      dbg("   Obtained address from base: " << register_name(base_register)
                                            << "\n");
    } else {
      auto pc_offset_11 = instruction.data() & 0x7ff;
      auto extended = sign_extend(pc_offset_11, 11);
      dbg("   Obtained address from offset: " << pc_offset_11 << "\n");
      address = get_register(Register::PC) + extended;
    }
    dbg("   Jumping to " << (const void *)address << "\n");

    // Then the PC is loaded with the address of the first instruction
    // of the subroutine, causing an unconditional jump to that address.
    set_register(Register::PC, address, ShouldUpdateCondition::No);

    break;
  }
  case OpCode::LD: {
    dbg("LD instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 0x7);

    dbg("   Destination: " << register_name(destination_register) << "\n");

    // An address is computed by sign-extending bits [8:0] to 16 bits and adding
    // this value to the incremented PC.
    auto pc_offset_9 = instruction.data() & 0x1ff;
    auto extended = sign_extend(pc_offset_9, 9);
    auto address = get_register(Register::PC) + extended;

    // The contents of memory at this address are loaded into DR.
    // The condition codes are set, based on whether the value loaded is
    // negative, zero, or positive
    set_register(destination_register, read_memory(address));

    break;
  }
  case OpCode::LDI: {
    dbg("LDI instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 0x07);

    dbg("   Destination: " << register_name(destination_register) << "\n");

    // An address is computed by sign-extending bits [8:0] to 16 bits and
    // adding this value to the incremented PC.
    auto pc_offset_9 = instruction.data() & 0x1ff;
    auto extended = sign_extend(pc_offset_9, 9);
    auto incremented_pc = get_register(Register::PC);
    auto indirect_address = extended + incremented_pc;

    dbg("   Indirect address: " << (const void *)indirect_address << "\n");

    // What is stored in memory at this address is the
    // address of the data to be loaded into DR.
    auto final_address = read_memory(indirect_address);

    dbg("   Final address: " << final_address << "\n");

    // The condition codes are set,
    // based on whether the value loaded is negative, zero, or positive.
    set_register(destination_register, read_memory(final_address));

    dbg("   Result: " << get_register(destination_register) << "\n");

    break;
  }
  case OpCode::LDR: {
    dbg("LDR Instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 07);

    dbg("   Destination: " << register_name(destination_register) << "\n");

    Register base_register =
        register_from_underlying((instruction.data() >> 6) & 07);

    dbg("   Base register: " << register_name(base_register) << "\n");

    // An address is computed by sign-extending bits [5:0] to 16 bits and
    // adding this value to the contents of the register specified by bits
    // [8:6].
    auto offset_6 = instruction.data() & 077;
    auto extended = sign_extend(offset_6, 6);
    auto address = get_register(base_register) + extended;

    dbg("   Computed address: " << (const void *)address << "\n");

    // The contents of memory at this address are loaded into DR.The
    // condition codes are set, based on whether the value loaded is
    // negative, zero, or positive.
    set_register(destination_register, read_memory(address));

    break;
  }
  case OpCode::LEA: {
    dbg("LEA Instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 07);

    dbg("   Destination: " << register_name(destination_register) << "\n");

    // An address is computed by sign-extending bits [8:0] to 16 bits and adding
    // this value to the incremented PC.
    auto pc_offset_9 = instruction.data() & 0x1ff;
    auto extended = sign_extend(pc_offset_9, 9);
    auto address = get_register(Register::PC) + extended;

    // This address is loaded into DR. The
    // condition codes are set, based on whether the value loaded is negative,
    // zero, or positive.
    set_register(destination_register, address);

    break;
  }
  case OpCode::NOT: {
    dbg("NOT Instruction\n");
    Register destination_register =
        register_from_underlying((instruction.data() >> 9) & 0x7);

    Register source_register =
        register_from_underlying((instruction.data() >> 6) & 0x7);

    dbg("   Destination: " << register_name(destination_register) << "\n");
    dbg("   Source: " << register_name(source_register) << "\n");

    // The bit-wise complement of the contents of SR is stored in DR.
    auto complement = ~get_register(source_register);

    // The condition codes are set, based on whether the binary value produced,
    // taken as a 2’s complement integer, is negative, zero, or positive.
    set_register(destination_register, complement);

    break;
  }
  case OpCode::RTI:
    dbg("Unused opcode");
    break;
  case OpCode::STI: {
    dbg("STI Instruction\n");
    Register source_register =
        register_from_underlying((instruction.data() >> 9) & 0x7);

    dbg("   Source: " << register_name(source_register) << "\n");

    // The contents of the register specified by SR are stored in the memory
    // location whose address is obtained as follows: Bits [8:0] are
    // sign-extended to 16 bits and added to the incremented PC.
    // What is in memory at this address is the address of the location to
    // which the data in SR is stored
    // NOTE: We follow mem[mem[PC † + SEXT(PCoffset9)]] = SR;
    auto pc_offset_9 = instruction.data() & 0x1ff;
    auto extended = sign_extend(pc_offset_9, 9);
    auto address = read_memory(get_register(Register::PC) + extended);
    auto contents = get_register(source_register);

    write_memory(address, contents);

    break;
  }
  case OpCode::ST: {
    dbg("ST Instruction\n");

    Register source_register =
        register_from_underlying((instruction.data() >> 9) & 0x7);

    dbg("   Source: " << register_name(source_register) << "\n");

    // The contents of the register specified by SR are stored in the memory
    // location whose address is computed by sign-extending bits [8:0] to 16
    // bits and adding this value to the incremented PC.
    auto pc_offset_9 = instruction.data() & 0x1ff;
    auto extended = sign_extend(pc_offset_9, 9);
    auto address = get_register(Register::PC) + extended;
    auto contents = get_register(source_register);

    write_memory(address, contents);

    break;
  }
  case OpCode::STR: {
    dbg("STR Instruction\n");
    Register source_register =
        register_from_underlying((instruction.data() >> 9) & 0x7);

    dbg("   Source: " << register_name(source_register) << "\n");

    Register base_register =
        register_from_underlying((instruction.data() >> 6) & 0x7);

    dbg("   Base: " << register_name(base_register) << "\n");

    // The contents of the register specified by SR are stored in the memory
    // location whose address is computed by sign-extending bits [5:0] to 16
    // bits and adding this value to the contents of the register specified
    // by bits [8:6]
    auto offset_6 = instruction.data() & 077;
    auto extended = sign_extend(offset_6, 6);
    auto address = get_register(base_register) + extended;

    write_memory(address, get_register(source_register));

    break;
  }
  case OpCode::TRAP: {
    dbg("TRAP Instruction\n");

    // First R7 is loaded with the incremented PC.
    // (This enables a return to the
    // instruction physically following the TRAP instruction in the original
    // program after the service routine has completed execution.)
    // set_register(Register::R7, get_register(Register::PC),
    // ShouldUpdateCondition::No);

    /// The starting address is contained in the memory
    // location whose address is obtained by zero-extending
    // trapvector8 to 16 bits
    uint16_t trap_vector_8 = instruction.data() & 0xff;
    // NOTE: Since we mask trap_vector_8 to only consider the lower
    // 8 bits, we already have zero extended it to 16 bits.
    uint16_t starting_address = read_memory(trap_vector_8);

    Trap trap = trap_from_underlying(trap_vector_8);

    // Then the PC is loaded with the starting address of the
    // system call specified by trapvector8.
    // set_register(Register::PC, starting_address,
    // ShouldUpdateCondition::No);
    switch (trap) {
    case Trap::GETC: {
      std::cout << "Trap::GETC\n";
      // Read a single character from the keyboard. The character
      // is not echoed onto the console. Its ASCII code is copied
      // into R0. The high eight bits of R0 are cleared.
      char ch;
      std::cin >> ch;
      uint16_t value = ch;

      set_register(Register::R0, value);

      break;
    }
    case Trap::OUT_: {
      std::cout << "Trap::OUT\n";
      // Write a character in R0[7:0] to the console display.
      auto r0 = get_register(Register::R0);
      char character = r0 & 0xff;
      std::cout << "OUT: " << character << std::endl;
      break;
    }
    case Trap::PUTS: {
      std::cout << "Trap::PUTS\n";
      // Write a string of ASCII characters to the console display.
      // The characters are contained
      // in consecutive memory locations, one character per memory
      // location, starting with the address specified in R0.
      // Writing terminates with the occurrence of x0000 in a
      // memory location
      auto address = get_register(Register::R0);
      char16_t current_char;
      while (current_char = read_memory(address), current_char != '\0') {
        std::cout << static_cast<char>(current_char);
        address++;
      }
      break;
    }
    case Trap::IN_: {
      std::cout << "Trap::IN\n";
      // Print a prompt on the screen and read a single character
      // from the keyboard. The character is echoed onto the
      // console monitor, and its ASCII code is copied into R0. The
      // high eight bits of R0 are cleared.
      std::cout << "> ";
      char ch;
      std::cin >> ch;
      std::cout << ch << std::flush;
      uint16_t value = ch;
      set_register(Register::R0, ch);

      break;
    }
    case Trap::PUTSP: {
      std::cout << "Trap::PUTSP\n";
      // Write a string of ASCII characters to the console. The
      // characters are contained in consecutive memory locations,
      // two characters per memory location, starting with the
      // address specified in R0.
      auto address = get_register(Register::R0);
      uint16_t current;
      // Writing terminates with the occurrence of x0000 in a
      // memory location
      while (current = read_memory(address), current != 0) {
        // The ASCII code contained in bits [7:0] of a memory
        // location is written to the console first.
        char ch = current & 0xff;

        std::cout << ch << std::flush;

        // Then the ASCII code contained in bits [15:8] of
        // that memory location is written to the console.
        // A character string
        // consisting of an odd number of characters to be written
        // will have x00 in bits [15:8] of the memory location
        // containing the last character to be written.)
        ch = (current >> 8);
        if (ch == 0)
          continue;
        dbg(ch);
      }

      break;
    }
    case Trap::HALT: {
      std::cout << "Trap::HALT\n";
      // Halt execution and print a message on the console.
      std::cout << "Program halted." << std::endl;
      std::exit(1);
      break;
    }
    }

    break;
  }
  default:
    dbg("Bad Opcode" << "\n");
    return ShouldBreak::Yes;
  }

  // FIXME: Break correctly
  return ShouldBreak::No;
}

Instruction VirtualMachine::current_instruction() {
  auto opcode_value = read_memory(m_registers[to_underlying(Register::PC)]);
  return {opcode_value};
}

uint16_t VirtualMachine::read_memory(uint16_t address) {
  if (address == MemoryMappedRegister::KBSR) {
    if (check_key()) {
      m_memory[MemoryMappedRegister::KBSR] = (1 << 15);
      std::cin >> m_memory[MemoryMappedRegister::KBDR];
    } else {
      m_memory[MemoryMappedRegister::KBSR] = 0;
    }
  }

  auto result = m_memory[address];
#if 0
  dbg("Reading address 0x" << (const void *)address << " from memory\n");
  dbg("   Result: " << result << "\n");
#endif
  return result;
}

void VirtualMachine::write_memory(uint16_t address, uint16_t value) {
  dbg("Storing value at address 0x" << (const void *)address << " in memory\n");
  dbg("   Value: " << value << "\n");
  m_memory[address] = value;
}

uint16_t VirtualMachine::get_register(Register reg) {
  return m_registers[to_underlying(reg)];
}

void VirtualMachine::set_register(
    Register reg, uint16_t val, ShouldUpdateCondition should_update_condition) {
  m_registers[to_underlying(reg)] = val;
  if (should_update_condition == ShouldUpdateCondition::Yes)
    update_flags(reg);
}

void VirtualMachine::update_flags(Register reg) {
  auto value = get_register(reg);
  if (value == 0) {
    set_condition_flag(ConditionFlag::ZRO);
  } else if ((value >> 15) == 1) {
    set_condition_flag(ConditionFlag::NEG);
  } else {
    set_condition_flag(ConditionFlag::POS);
  }
}

void VirtualMachine::set_condition_flag(ConditionFlag flag) {
  m_registers[to_underlying(Register::COND)] = to_underlying(flag);
}

void VirtualMachine::dump_registers() {
  std::cout << "=====Registers============\n";
  for (size_t i = 0; i < to_underlying(Register::COUNT); i++) {
    auto reg = static_cast<Register>(i);
    std::cout << register_name(reg) << " = " << m_registers[i] << " (hex: 0x"
              << (const void *)m_registers[i] << ")" << "\n";
  }
  std::cout << "=========================\n";
}

void VirtualMachine::dump_memory() {
  std::cout << "=======Memory=========\n";
  for (size_t i = 0; i < MEMORY_MAX; i++) {
    auto result = m_memory[i];
    if (result == 0)
      continue;
    std::cout << "0x" << (const void *)i << ": " << result
              << ", neg: " << (int16_t)result << "(" << Instruction(m_memory[i])
              << "), " << "\n";
  }
  std::cout << "======================\n";
}

uint16_t VirtualMachine::sign_extend(uint16_t x, int bit_count) {
  // If x has a 1 in the bit_count's position, then its negative
  if ((x >> (bit_count - 1)) & 1) {
    // Pad with 1 if it's negative
    x |= (0xffff << bit_count);
  }
  return x;
}