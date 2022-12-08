#pragma once

#include "exprs/exprs.hh"

struct UnaryExpr : public Expr {
  explicit UnaryExpr(enum Type type) : Expr(type) {}
  explicit UnaryExpr(enum Type type, std::shared_ptr<Expr> expr)
      : Expr(type), expr_(std::move(expr)) {}

  auto Append(std::shared_ptr<Expr> expr) -> void final {
    if (expr_) {
      SetError();
      return;
    }

    expr_ = std::move(expr);
  }

  auto Append(enum Type type) -> void final {
    (void)type;
    SetError();
  }

  [[nodiscard]] auto Complete() const -> bool final {
    return Type() != Type::kNull && expr_;
  }

  auto Accept(ExprVisitor &visitor) const -> void override {
    visitor.Visit(*this);
  }

  std::shared_ptr<Expr> expr_{};
};

// A Special UnaryExpr Expx where E is a quantifier
// Need to handle this case (by checking an additional Token)
struct QuantifiedUnaryExpr : public UnaryExpr {
public:
  explicit QuantifiedUnaryExpr(enum Type type, Token var)
      : UnaryExpr(type), var_(std::move(var)) {}
  explicit QuantifiedUnaryExpr(enum Type type, Token var,
                               std::shared_ptr<Expr> expr)
      : UnaryExpr(type, std::move(expr)), var_(std::move(var)) {}

  auto Accept(ExprVisitor &visitor) const -> void final {
    visitor.Visit(*this);
  }

  Token var_;
};
