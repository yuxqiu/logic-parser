#include "constant.hh"

Constant::Constant(uint64_t constant) : constant_{constant} {}

auto operator<(const Constant &lhs, const Constant &rhs) -> bool {
  return lhs.constant_ < rhs.constant_;
}