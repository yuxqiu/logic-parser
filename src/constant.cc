#include "constant.hh"
#include <optional>

auto ConstantManager::GetConsts(uint64_t num) const -> std::optional<Token> {
  if (num >= kLimit || num >= generated_constants_.size()) {
    return {};
  }
  return generated_constants_[num];
}

auto ConstantManager::CanAddConst() const -> bool {
  return generated_constants_.size() < kLimit;
}