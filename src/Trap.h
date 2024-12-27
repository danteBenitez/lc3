#pragma once

#include <Utils.h>
#include <numeric>

enum class Trap {
  GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
  OUT_ = 0x21,  /* output a character */
  PUTS = 0x22,  /* output a word string */
  IN_ = 0x23,   /* get character from keyboard, echoed onto the terminal */
  PUTSP = 0x24, /* output a byte string */
  HALT = 0x25   /* halt the program */
};

class InvalidTrap {};

inline Trap trap_from_underlying(uint16_t value) {
  if (value > to_underlying(Trap::HALT) || value < to_underlying(Trap::GETC)) {
    throw InvalidTrap();
  }
  return static_cast<Trap>(value);
}

inline const char *trap_name(Trap trap) {
  switch (trap) {
  case Trap::GETC: {
    return "Trap::GETC";
  }
  case Trap::OUT_: {
    return "Trap::OUT";
  }
  case Trap::PUTS: {
    return "Trap::PUTS";
  }
  case Trap::IN_: {
    return "Trap::IN";
  }
  case Trap::PUTSP: {
    return "Trap::PUTSP";
  }
  case Trap::HALT: {
    return "Trap::HALT";
  }
  }
}