#pragma once

enum class OpCode {
  BR = 0,
  ADD,
  LD,
  ST,
  JSR,
  AND,
  LDR,
  STR,
  RTI,
  NOT,
  LDI,
  STI,
  JMP,
  RES,
  LEA,
  TRAP,
  COUNT
};

inline const char *opcode_name(OpCode const &op) {
  switch (op) {
  case OpCode::BR:
    return "OpCode::BR";
  case OpCode::ADD:
    return "OpCode::ADD";
  case OpCode::LD:
    return "OpCode::LD";
  case OpCode::ST:
    return "OpCode::ST";
  case OpCode::JSR:
    return "OpCode::JSR";
  case OpCode::AND:
    return "OpCode::AND";
  case OpCode::LDR:
    return "OpCode::LDR";
  case OpCode::STR:
    return "OpCode::STR";
  case OpCode::RTI:
    return "OpCode::RTI";
  case OpCode::NOT:
    return "OpCode::NOT";
  case OpCode::LDI:
    return "OpCode::LDI";
  case OpCode::STI:
    return "OpCode::STI";
  case OpCode::JMP:
    return "OpCode::JMP";
  case OpCode::RES:
    return "OpCode::RES";
  case OpCode::LEA:
    return "OpCode::LEA";
  case OpCode::TRAP:
    return "OpCode::TRAP";
  }
}
