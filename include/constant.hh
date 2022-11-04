#pragma once

#include <cstdint>

class Constant {
public:
  explicit Constant(uint64_t constant);

private:
  uint64_t constant_;

  friend auto operator<(const Constant &lhs, const Constant &rhs) -> bool;
};

auto operator<(const Constant &lhs, const Constant &rhs) -> bool;