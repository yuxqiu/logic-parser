#include "constant.hh"
#include "tokenizer.hh"
#include <optional>

auto ConstantManager::GetConsts(uint64_t num) -> std::optional<Token> {
  // To circumvent the situation that we have no constant, which means
  //  - no existential formula
  //
  // In this case, we generate a new constant instead
  // and see if the generated theory will be closed (or open)
  if (generated_constants_.empty()) {
    AddConst();
  }

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