#pragma once

#include <numeric>

enum class ConditionFlag { POS = 1 << 0, ZRO = 1 << 1, NEG = 1 << 2 };

enum class Register : uint16_t {
  R0 = 0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  PC,
  COND,
  COUNT
};

class InvalidRegister {};

inline Register register_from_underlying(uint16_t value) {
  if (value >= to_underlying(Register::COUNT)) {
    throw InvalidRegister();
  }
  return static_cast<Register>(value);
}

inline const char *register_name(Register reg) {
  switch (reg) {
  case Register::R0:
    return "Register::R0";
    break;
  case Register::R1:
    return "Register::R1";
  case Register::R2:
    return "Register::R2";
  case Register::R3:
    return "Register::R3";
  case Register::R4:
    return "Register::R4";
  case Register::R5:
    return "Register::R5";
  case Register::R6:
    return "Register::R6";
  case Register::R7:
    return "Register::R7";
  case Register::PC:
    return "Register::PC";
  case Register::COND:
    return "Register::COND";
  case Register::COUNT:
    return "Register::COUNT";
  default:
    return "Unrecognized";
    break;
  }
}