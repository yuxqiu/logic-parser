#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "tokenizer.hh"

class ConstantManager {
public:
  // If number is >= generated_constants_.size() or is >= klimit
  // return nullopt
  //
  // Generate a new const if currently there is no const
  [[nodiscard]] auto GetConsts(uint64_t num) -> std::optional<Token>;

  // If we can add more const
  [[nodiscard]] auto CanAddConst() const -> bool;

  // Add a new constant and Return it
  auto AddConst() -> Token;

private:
  std::vector<Token> generated_constants_{};
  constexpr static uint64_t kLimit{10};
};