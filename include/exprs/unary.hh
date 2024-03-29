#pragma once

#include "exprs/expr.hh"

struct UnaryExpr : public Expr {
  explicit UnaryExpr(ExprKind type) : Expr(type) {}
  explicit UnaryExpr(ExprKind type, std::shared_ptr<Expr> expr)
      : Expr(type), expr_(std::move(expr)) {}

  auto Append(std::shared_ptr<Expr> expr) -> void final {
    if (expr_) {
      SetError();
      return;
    }

    expr_ = std::move(expr);
  }

  auto Append(ExprKind type) -> void final {
    (void)type;
    SetError();
  }

  [[nodiscard]] auto Complete() const -> bool final {
    return Type() != ExprKind::kNull && expr_;
  }

  auto Accept(ExprVisitor &visitor) const -> void override {
    visitor.Visit(*this);
  }

  std::shared_ptr<Expr> expr_{};
};

// A Special UnaryExpr Expr where E is a quantifier
// Need to handle this case (by checking an additional Token)
struct QuantifiedUnaryExpr final : public UnaryExpr {
public:
  explicit QuantifiedUnaryExpr(ExprKind type, Token var)
      : UnaryExpr(type), var_(std::move(var)) {}
  explicit QuantifiedUnaryExpr(ExprKind type, Token var,
                               std::shared_ptr<Expr> expr)
      : UnaryExpr(type, std::move(expr)), var_(std::move(var)) {}

  auto Accept(ExprVisitor &visitor) const -> void final {
    visitor.Visit(*this);
  }

  Token var_;
};
