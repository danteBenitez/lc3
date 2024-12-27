#pragma once

#include <Instruction.h>
#include <Register.h>
#include <Utils.h>
#include <numeric>

class VirtualMachine {
public:
  VirtualMachine();
  ~VirtualMachine();

  void execute();
  Instruction current_instruction();

  enum class ShouldUpdateCondition { Yes, No };

  uint16_t *base() { return m_memory; }

  uint16_t get_register(Register);
  void set_register(Register, uint16_t,
                    ShouldUpdateCondition = ShouldUpdateCondition::Yes);

  enum class ShouldBreak { Yes, No };

  ShouldBreak perform(Instruction);

  uint16_t sign_extend(uint16_t, int bit_count);

  void update_flags(Register value);
  void set_condition_flag(ConditionFlag);

  uint16_t read_memory(uint16_t address);
  void write_memory(uint16_t address, uint16_t value);

  void dump_registers();
  void dump_memory();

  static constexpr size_t MEMORY_MAX = 1 << 16;
  static constexpr size_t PC_START = 0x3000;

  void copy_memory_from(const uint16_t *mem) {
    std::memcpy(m_memory, mem, MEMORY_MAX * sizeof(uint16_t));
  }

private:
  uint16_t m_memory[MEMORY_MAX] = {0};
  uint16_t m_registers[to_underlying(Register::COUNT)] = {0};
};