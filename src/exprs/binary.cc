#include "exprs/binary.hh"
#include "expr.hh"
#include "exprs/unary.hh"

BinaryExpr::BinaryExpr(enum Type type, std::shared_ptr<Expr> expr_lhs,
                       std::shared_ptr<Expr> expr_rhs)
    : Expr(type), expr_lhs_(std::move(expr_lhs)),
      expr_rhs_(std::move(expr_rhs)) {}

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
  if (type != Type::kAnd && type != Type::kOr && type != Type::kImpl) {
    SetError();
    return;
  }

  type_ = type;
}

[[nodiscard]] auto BinaryExpr::ViewChildren() const
    -> std::vector<std::shared_ptr<Expr>> {
  return {expr_lhs_, expr_rhs_};
}

[[nodiscard]] auto BinaryExpr::Complete() const -> bool {
  return type_ != Type::kNull && expr_lhs_ && expr_rhs_;
}

[[nodiscard]] auto BinaryExpr::Infos() const -> std::vector<Token> {
  return {};
}