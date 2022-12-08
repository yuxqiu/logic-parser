#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "tokenizer.hh"

class ConstantManager {
public:
  // If number is >= generated_constants_.size() return nullopt
  [[nodiscard]] auto GetConsts(uint64_t num) -> std::optional<Token> {
    if (num >= generated_constants_.size()) {
      return {};
    }
    return generated_constants_[num];
  }

  // If we can add more const
  [[nodiscard]] auto CanAddConst() const -> bool {
    return generated_constants_.size() < kLimit;
  }

  // Add a new constant and Return it
  auto AddConst() -> Token {
    return generated_constants_.emplace_back(
        std::to_string(generated_constants_.size()));
  }

private:
  std::vector<Token> generated_constants_{};
  constexpr static uint64_t kLimit{10};
};