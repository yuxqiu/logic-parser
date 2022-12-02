#include "exprs/binary.hh"
#include "expr.hh"
#include "exprs/unary.hh"

auto BinaryExpr::Append(std::shared_ptr<Expr> expr) -> void {
  if (expr_lhs_ && expr_rhs_) {
    SetError();
    return;
  }

  if (!expr_lhs_) {
    expr_lhs_ = expr;
    return;
  }

  if (type_ == Type::kNull) {
    SetError();
    return;
  }

  expr_rhs_ = std::move(expr);
}

auto BinaryExpr::Append(enum Type type) -> void {
  if (type_ != Type::kNull || !expr_lhs_) {
    SetError();
    return;
  }
  if (!IsBinary(type)) {
    SetError();
    return;
  }

  type_ = type;
}
