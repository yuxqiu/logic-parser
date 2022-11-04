#include <cassert>
#include <memory>
#include <ostream>
#include <string>

#include "expr.hh"
#include "tokenizer.hh"

auto Expr::Error() const -> bool { return error_; }

void Expr::SetError() { error_ = true; }

auto Expr::Type() const -> enum Type { return type_; }

auto Expr::IsLiteral(enum Expr::Type type) -> bool {
  return type == Expr::Type::kLiteral;
}

auto Expr::IsUnary(enum Expr::Type type) -> bool {
  return type == Expr::Type::kNeg || type == Expr::Type::kExist ||
         type == Expr::Type::kUniversal;
}

auto Expr::IsBinary(enum Expr::Type type) -> bool {
  return type == Expr::Type::kAnd || type == Expr::Type::kImpl ||
         type == Expr::Type::kOr;
}

auto operator<(const Expr &lhs, const Expr &rhs) -> bool {
  return lhs.type_ < rhs.type_;
}

auto operator<<(std::ostream &out, enum Expr::Type type) -> std::ostream & {
  switch (type) {
  case Expr::Type::kAnd:
    out << "^";
    break;
  case Expr::Type::kOr:
    out << "v";
    break;
  case Expr::Type::kImpl:
    out << ">";
    break;
  default:
    assert(false);
  }

  return out;
}

Literal::Literal(Token val) : val_{std::move(val)} { type_ = Type::kLiteral; }

void Literal::Append(std::shared_ptr<Expr> expr) {
  (void)expr;
  SetError();
}

void Literal::Append(enum Type type) {
  (void)type;
  SetError();
}

[[nodiscard]] auto Literal::TakeChildren()
    -> std::vector<std::shared_ptr<Expr>> {
  return {};
}

[[nodiscard]] auto Literal::ViewChildren() const
    -> std::vector<std::shared_ptr<Expr>> {
  return {};
}

[[nodiscard]] auto Literal::Complete() const -> bool { return true; }

auto Literal::Description() const -> std::string { return val_.ToString(); }

PredicateLiteral::PredicateLiteral(Token val, Token left, Token right)
    : Literal(std::move(val)), left_var_(std::move(left)),
      right_var_(std::move(right)) {}

auto PredicateLiteral::Description() const -> std::string {
  return Literal::Description() + "(" + left_var_.ToString() + "," +
         right_var_.ToString() + ")";
}

UnaryExpr::UnaryExpr(enum Type type) {
  if (type != Type::kExist && type != Type::kNeg && type != Type::kUniversal) {
    SetError();
    return;
  }
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

[[nodiscard]] auto UnaryExpr::TakeChildren()
    -> std::vector<std::shared_ptr<Expr>> {
  return {std::move(expr_)};
}

[[nodiscard]] auto UnaryExpr::ViewChildren() const
    -> std::vector<std::shared_ptr<Expr>> {
  return {expr_};
}

[[nodiscard]] auto UnaryExpr::Complete() const -> bool {
  return type_ != Type::kNull && expr_;
}

auto UnaryExpr::Description() const -> std::string {
  switch (type_) {
  case Expr::Type::kNeg:
    return "-" + expr_->Description();
  case Expr::Type::kExist:
    [[fallthrough]];
  case Expr::Type::kUniversal:
    return expr_->Description();
  default:
    break;
  }

  assert(false);
  return "Unreachable";
}

QuantifiedUnaryExpr::QuantifiedUnaryExpr(enum Type type, Token var)
    : UnaryExpr(type), var_(std::move(var)) {}

auto QuantifiedUnaryExpr::Description() const -> std::string {
  switch (type_) {
  case Expr::Type::kExist:
    return "E" + var_.ToString() + UnaryExpr::Description();
  case Expr::Type::kUniversal:
    return "A" + var_.ToString() + UnaryExpr::Description();
  default:
    break;
  }

  assert(false);
  return "Unreachable";
}

void BinaryExpr::Append(std::shared_ptr<Expr> expr) {
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

void BinaryExpr::Append(enum Type type) {
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

[[nodiscard]] auto BinaryExpr::TakeChildren()
    -> std::vector<std::shared_ptr<Expr>> {
  return {std::move(expr_lhs_), std::move(expr_rhs_)};
}

[[nodiscard]] auto BinaryExpr::ViewChildren() const
    -> std::vector<std::shared_ptr<Expr>> {
  return {expr_lhs_, expr_rhs_};
}

[[nodiscard]] auto BinaryExpr::Complete() const -> bool {
  return type_ != Type::kNull && expr_lhs_ && expr_rhs_;
}

auto BinaryExpr::Description() const -> std::string {
  switch (type_) {
  case Expr::Type::kAnd:
    return "(" + expr_lhs_->Description() + "^" + expr_rhs_->Description() +
           ")";
  case Expr::Type::kOr:
    return "(" + expr_lhs_->Description() + "v" + expr_rhs_->Description() +
           ")";
  case Expr::Type::kImpl:
    return "(" + expr_lhs_->Description() + ">" + expr_rhs_->Description() +
           ")";
  default:
    break;
  }

  assert(false);
  return "Unreachable";
}