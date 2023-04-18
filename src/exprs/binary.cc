#include "exprs/binary.hh"

auto BinaryExpr::Append(std::shared_ptr<Expr> expr) -> void {
  if (expr_lhs_ && expr_rhs_) {
    SetError();
    return;
  }

  if (!expr_lhs_) {
    expr_lhs_ = std::move(expr);
    return;
  }

  if (Type() == ExprKind::kNull) {
    SetError();
    return;
  }

  expr_rhs_ = std::move(expr);
}

auto BinaryExpr::Append(ExprKind type) -> void {
  if (Type() != ExprKind::kNull || !expr_lhs_ || !ExprKind::IsBinary(type)) {
    SetError();
    return;
  }
  SetType(type);
}
