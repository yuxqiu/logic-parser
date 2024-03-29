#pragma once

#include <cstddef>
#include <string>

class Token {
public:
  explicit Token() = default;
  explicit Token(std::string token) : token_{std::move(token)} {}
  [[nodiscard]] auto ToString() const -> std::string { return token_; }

private:
  std::string token_{};

  friend auto operator<(const Token &lhs, const Token &rhs) -> bool {
    return lhs.token_ < rhs.token_;
  }

  friend auto operator==(const Token &lhs, const Token &rhs) -> bool {
    return lhs.token_ == rhs.token_;
  }
};

namespace std {
template <> struct hash<Token> {
  auto operator()(const Token &token) const -> size_t {
    return hash<std::string>()(token.ToString());
  }
};
} // namespace std

class Tokenizer {
public:
  explicit Tokenizer(std::string expr) : expr_{std::move(expr)} {
    ConsumeWhitespace();
  }

  [[nodiscard]] auto PeekToken() const -> Token {
    return Token{std::string{expr_[start_]}};
  }

  void PopToken() {
    ++start_;
    ConsumeWhitespace();
  }

  [[nodiscard]] auto Empty() const -> bool {
    return start_ == std::string::npos;
  }

private:
  void ConsumeWhitespace() {
    const static char *k_whitespaces = " \t\n\v\f\r";
    start_ = expr_.find_first_not_of(k_whitespaces, start_);
  }

  std::string expr_;
  std::string::size_type start_{0};
};