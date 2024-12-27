#pragma once
#include <iostream>

template <typename EnumLike> constexpr size_t to_underlying(EnumLike v) {
  return static_cast<size_t>(v);
}

template <typename... Args> std::ostream &print_hexadecimal(Args... args) {
  auto flags = std::cout.flags();
#ifdef VM_DEBUG
  std::cout << std::hex << (args << ...);
  std::cout.flags(flags);
#endif
  return std::cout;
}