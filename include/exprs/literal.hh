#pragma once

#include "exprs/exprs.hh"

class Literal : public Expr {
public:
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

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> final {
    return {};
  }

  [[nodiscard]] auto Infos() const -> std::vector<Token> override {
    return {val_};
  }

protected:
  Token val_;
};

class PredicateLiteral : public Literal {
public:
  explicit PredicateLiteral(Token val, Token left, Token right)
      : Literal(std::move(val)), left_var_(std::move(left)),
        right_var_(std::move(right)) {}

  [[nodiscard]] auto Infos() const -> std::vector<Token> final {
    return {val_, left_var_, right_var_};
  }

  Token left_var_, right_var_;
};