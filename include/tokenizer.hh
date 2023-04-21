#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class Token {
public:
  explicit Token() = default;
  explicit Token(std::string token) : token_{std::move(token)} {}

  // because we want to be able to compare with string_view/string implicitly
  [[nodiscard]] operator std::string_view() const { return token_; } // NOLINT

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
    return hash<std::string_view>()(std::string_view{token});
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
