#define VM_DEBUG

#include <Platform.h>
#include <VirtualMachine.h>
#include <iostream>

void handle_interrupt(int signal) {
  restore_input_buffering();
  std::cout << "\n";
  exit(-2);
}

void setup() {
  signal(SIGINT, handle_interrupt);
  disable_input_buffering();
}

void teardown() { restore_input_buffering(); }

uint16_t swap16(uint16_t value) { return (value << 8) | (value >> 8); }

void execute_image(FILE *file, VirtualMachine &vm) {
  // the origin tells where in memory to place the image
  uint16_t origin;
  fread(&origin, sizeof(origin), 1, file);
  origin = swap16(origin);

  // we know the maximum file size so we only need one fread
  uint16_t max_read = VirtualMachine::MEMORY_MAX - origin;
  uint16_t *p = vm.base() + origin;
  size_t read = fread(p, sizeof(uint16_t), max_read, file);

  while (read-- < 0) {
    *p = swap16(*p);
    ++p;
  }

  vm.dump_memory();
  vm.execute();
}

void run_example(VirtualMachine &vm) {

  vm.dump_registers();

  uint16_t program[VirtualMachine::MEMORY_MAX] = {0};

  /*
  // ADD R0, R0, 2
  program[VirtualMachine::PC_START] = 0b0001'0000'0010'0010;
  // ADD R0, R0, 1
  program[VirtualMachine::PC_START + 1] = 0b0001'0000'0010'0001;
  */
  // TRAP IN
  program[0x3000] = 0xf023;

  // JSR to_int(R0)
  program[0x3001] = 0x480A;

  // The char is in R0, copy it to memory
  // ST R0 7
  program[0x3002] = 0x3007;

  // Grab another char
  // TRAP IN
  program[0x3003] = 0xf023;

  // JSR to_int(R0)
  program[0x3004] = 0x4807;

  // Load the first char (now in memory) into R1
  // LD R1, 4
  program[0x3005] = 0x2204;

  // Add R0 and R1
  // ADD R0, R0, R1
  program[0x3006] = 0b0001'000'000'0'00'001;

  // JSR to_char(R0)
  program[0x3007] = 0x480A;

  // Print the result that is in R0
  // TRAP PUT
  program[0x3008] = 0xf021;

  // TRAP HALT
  program[0x3009] = 0xf025;

  // program[0x300A] = (the char is copied here)

  /// Subroutines
  // to_int(R0)

  // LD R5, 10
  std::cout << "0x3000C: to_int(R0)\n";
  program[0x300C] = 0b0010'101'000001010;

  // ADD R0, R0, R5
  program[0x300D] = 0b0001'000'000'0'00'101;

  // RET
  program[0x300E] = 0b1100'000'111'000000;

  // to_char(R0)
  // LD R5, 10
  std::cout << "0x3012: to_char(R0)\n";
  program[0x3012] = 0b0010'101'000001010;

  // ADD R0, R0, R5
  program[0x3013] = 0b0001'000'000'0'00'101;

  // RET
  program[0x3014] = 0b1100'000'111'000000;

  // Here we store data for the to_int(R0) subroutine
  // Since the 0 char is 48 in ASCII, we need to substract it
  // to obtain the digit's value.
  program[0x3017] = (~48 + 1); // Two's complement of 48

  // Here we store data for the to_char(R0) subroutine
  // We do the inverse here, we add 48.
  program[0x301D] = 48;

  vm.copy_memory_from(program);
  vm.dump_memory();

  // std::cout << "Program: " << program[VirtualMachine::PC_START] << "\n";
  vm.execute();
}

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cout << "Usage: vm <image-paths...>\n" << std::endl;
    return 2;
  }

  setup();

  VirtualMachine vm;

  for (size_t i = 1; i < argc; i++) {
    auto filepath = argv[i];
    if (strcmp(filepath, "example") == 0) {
      run_example(vm);
      continue;
    }
    FILE *file;
    auto rc = fopen_s(&file, filepath, "r");
    if (rc != 0) {
      std::cout << "Error: " << errno;
      break;
    }
    std::cout << "Executing: " << filepath << " image\n";
    execute_image(file, vm);
    fclose(file);
  }

  teardown();
  return 0;
}
