#include "constant.hh"
#include "tokenizer.hh"
#include <optional>

auto ConstantManager::GetConsts(uint64_t num) -> std::optional<Token> {
  if (num >= generated_constants_.size()) {
    return {};
  }
  return generated_constants_[num];
}

auto ConstantManager::CanAddConst() const -> bool {
  return generated_constants_.size() < kLimit;
}

auto ConstantManager::AddConst() -> Token {
  generated_constants_.emplace_back(
      std::to_string(generated_constants_.size()));
  return generated_constants_.back();
}