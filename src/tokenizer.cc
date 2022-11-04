#include <string>
#include <utility>

#include "tokenizer.hh"

Token::Token(std::string token) : token_{std::move(token)} {}

auto Token::ToString() const -> std::string { return token_; }

Tokenizer::Tokenizer(std::string expr) : expr_{std::move(expr)} {
  ConsumeWhitespace();
}

[[nodiscard]] auto Tokenizer::PeekToken() const -> Token {
  return Token{std::string{expr_[start_]}};
}

void Tokenizer::PopToken() {
  ++start_;
  ConsumeWhitespace();
}

auto Tokenizer::Empty() const -> bool { return start_ == expr_.size(); }

void Tokenizer::ConsumeWhitespace() {
  const char *whitespaces = " \t\n\v\f\r";
  start_ = expr_.find_first_not_of(whitespaces, start_);
  start_ = start_ == std::string::npos ? expr_.size() : start_;
}

auto operator==(const Token &lhs, const Token &rhs) -> bool {
  return lhs.token_ == rhs.token_;
}

auto operator<(const Token &lhs, const Token &rhs) -> bool {
  return lhs.token_ < rhs.token_;
}