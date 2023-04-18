#pragma once

#include <memory>

#include "exprs/kind.hh"
#include "tokenizer.hh"

class Expr;

struct Literal {
public:
  explicit Literal(Token val) : val_{std::move(val)} {}

  [[nodiscard]] static auto Type() -> ExprKind { return ExprKind::kLiteral; }

  auto Append(const std::shared_ptr<Expr> &expr) -> void {
    (void)expr;
    error_ = true;
  }

  auto Append(ExprKind type) -> void {
    (void)type;
    error_ = true;
  }

  [[nodiscard]] static auto Complete() -> bool { return true; }

  Token val_;
  bool error_ = false;
};

struct PredicateLiteral final : public Literal {
public:
  explicit PredicateLiteral(Token val, Token left, Token right)
      : Literal(std::move(val)), left_var_(std::move(left)),
        right_var_(std::move(right)) {}

  Token left_var_, right_var_;
};
