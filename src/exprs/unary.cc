#include "exprs/unary.hh"
#include "exprs/binary.hh"

UnaryExpr::UnaryExpr(enum Type type) {
  if (type != Type::kExist && type != Type::kNeg && type != Type::kUniversal) {
    SetError();
    return;
  }
  type_ = type;
}

UnaryExpr::UnaryExpr(enum Type type, std::shared_ptr<Expr> expr)
    : expr_(std::move(expr)) {
  type_ = type;
}

void UnaryExpr::Append(std::shared_ptr<Expr> expr) {
  if (expr_ || type_ == Type::kNull) {
    SetError();
    return;
  }

  expr_ = std::move(expr);
}

void UnaryExpr::Append(enum Type type) {
  (void)type;
  SetError();
}

[[nodiscard]] auto UnaryExpr::ViewChildren() const
    -> std::vector<std::shared_ptr<Expr>> {
  return {expr_};
}

[[nodiscard]] auto UnaryExpr::Infos() const -> std::vector<Token> { return {}; }

[[nodiscard]] auto UnaryExpr::Complete() const -> bool {
  return type_ != Type::kNull && expr_;
}

QuantifiedUnaryExpr::QuantifiedUnaryExpr(enum Type type, Token var)
    : UnaryExpr(type), var_(std::move(var)) {}

QuantifiedUnaryExpr::QuantifiedUnaryExpr(enum Type type, Token var,
                                         std::shared_ptr<Expr> expr)
    : UnaryExpr(type, std::move(expr)), var_(std::move(var)) {}

[[nodiscard]] auto QuantifiedUnaryExpr::Infos() const -> std::vector<Token> {
  return {var_};
}