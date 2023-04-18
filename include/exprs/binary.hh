#pragma once

#include "exprs/expr.hh"

struct BinaryExpr final : public Expr {
  BinaryExpr() = default;
  explicit BinaryExpr(ExprKind type, std::shared_ptr<Expr> expr_lhs,
                      std::shared_ptr<Expr> expr_rhs)
      : Expr(type), expr_lhs_(std::move(expr_lhs)),
        expr_rhs_(std::move(expr_rhs)) {}

  auto Append(std::shared_ptr<Expr> expr) -> void final;
  auto Append(ExprKind type) -> void final;

  [[nodiscard]] auto Complete() const -> bool final {
    return Type() != ExprKind::kNull && expr_lhs_ && expr_rhs_;
  }

  auto Accept(ExprVisitor &visitor) const -> void final {
    visitor.Visit(*this);
  }

  std::shared_ptr<Expr> expr_lhs_{}, expr_rhs_{};
};
