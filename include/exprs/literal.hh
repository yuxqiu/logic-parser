#pragma once

#include "exprs/exprs.hh"

struct Literal : public Expr {
  explicit Literal(Token val) : Expr(Type::kLiteral), val_{std::move(val)} {}

  auto Append(std::shared_ptr<Expr> expr) -> void final {
    (void)expr;
    SetError();
  }

  auto Append(enum Type type) -> void final {
    (void)type;
    SetError();
  }

  [[nodiscard]] auto Complete() const -> bool final { return true; }

  auto Accept(ExprVisitor &visitor) const -> void override {
    visitor.Visit(*this);
  }

  Token val_;
};

struct PredicateLiteral final : public Literal {
  explicit PredicateLiteral(Token val, Token left, Token right)
      : Literal(std::move(val)), left_var_(std::move(left)),
        right_var_(std::move(right)) {}

  auto Accept(ExprVisitor &visitor) const -> void final {
    visitor.Visit(*this);
  }

  Token left_var_, right_var_;
};