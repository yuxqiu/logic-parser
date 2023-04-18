#pragma once

#include <memory>

#include "exprs/kind.hh"

class Expr;

struct BinaryExpr {
public:
  explicit BinaryExpr() = default;
  explicit BinaryExpr(ExprKind type, std::shared_ptr<Expr> expr_lhs,
                      std::shared_ptr<Expr> expr_rhs)
      : expr_lhs_(std::move(expr_lhs)), expr_rhs_(std::move(expr_rhs)),
        type_(type) {}

  [[nodiscard]] auto Type() const -> ExprKind { return type_; }

  auto Append(std::shared_ptr<Expr> expr) -> void {
    if (expr_lhs_ && expr_rhs_) {
      error_ = true;
      return;
    }

    if (!expr_lhs_) {
      expr_lhs_ = std::move(expr);
      return;
    }

    if (type_ == ExprKind::kNull) {
      error_ = true;
      return;
    }

    expr_rhs_ = std::move(expr);
  }
  auto Append(ExprKind type) -> void {
    if (!expr_lhs_ || !ExprKind::IsBinary(type)) {
      error_ = true;
      return;
    }
    type_ = type;
  }

  [[nodiscard]] auto Complete() const -> bool {
    return type_ != ExprKind::kNull && expr_lhs_ && expr_rhs_;
  }

  std::shared_ptr<Expr> expr_lhs_{}, expr_rhs_{};
  ExprKind type_;
  bool error_ = false;
};
