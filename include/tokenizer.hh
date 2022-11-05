#pragma once

#include <cstddef>
#include <string>

class Token {
public:
  explicit Token(std::string token);
  [[nodiscard]] auto ToString() const -> std::string;

private:
  std::string token_;

  friend auto operator<(const Token &lhs, const Token &rhs) -> bool;
  friend auto operator==(const Token &lhs, const Token &rhs) -> bool;
};

namespace std {
template <> struct hash<Token> {
  auto operator()(const Token &token) const -> size_t;
};
} // namespace std

auto operator<(const Token &lhs, const Token &rhs) -> bool;
auto operator==(const Token &lhs, const Token &rhs) -> bool;

class Tokenizer {
public:
  explicit Tokenizer(std::string expr);

  [[nodiscard]] auto PeekToken() const -> Token;
  void PopToken();
  [[nodiscard]] auto Empty() const -> bool;

private:
  void ConsumeWhitespace();

  std::string::size_type start_{0};
  std::string expr_;
};