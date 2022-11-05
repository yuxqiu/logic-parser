#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "tokenizer.hh"

class ConstantManager {
public:
  // If number is >= generated_constants_.size() or is >= klimit
  // return nullopt
  [[nodiscard]] auto GetConsts(uint64_t num) const -> std::optional<Token>;

  // If we can add more const
  [[nodiscard]] auto CanAddConst() const -> bool;

private:
  std::vector<Token> generated_constants_{};
  constexpr static uint64_t kLimit{10};
};