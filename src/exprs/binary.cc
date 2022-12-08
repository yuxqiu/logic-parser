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

  if (Type() == Type::kNull) {
    SetError();
    return;
  }

  expr_rhs_ = std::move(expr);
}

auto BinaryExpr::Append(enum Type type) -> void {
  if (Type() != Type::kNull || !expr_lhs_ || !IsBinary(type)) {
    SetError();
    return;
  }
  SetType(type);
}
