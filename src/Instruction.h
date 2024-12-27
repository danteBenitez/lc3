#pragma once

#include <Opcode.h>
#include <Utils.h>
#include <iostream>
#include <numeric>

class Instruction {
public:
  class InvalidInstruction : public std::exception {
    const char *what() { return "Invalid Instruction found"; }
  };

  Instruction(uint16_t data) : m_data(data) {}

  const OpCode opcode() const {
    uint16_t value = m_data >> 12;
    if (value >= to_underlying(OpCode::COUNT)) {
      throw InvalidInstruction();
    }
    return static_cast<OpCode>(value);
  }

  const uint16_t data() const { return m_data; }

  const uint16_t params() const { return m_data & 0x0fff; }

  friend std::ostream &operator<<(std::ostream &os,
                                  const Instruction &instruction) {
    return os << "Instruction[opcode: " << opcode_name(instruction.opcode())
              << ", data: " << instruction.data()
              << ", params: " << instruction.params() << "]";
  }

private:
  uint16_t m_data;
};
