#include <cassert>
#include <memory>
#include <string>

#include "expr.hh"
#include "tokenizer.hh"

auto Expr::Error() const -> bool { return error_; }

void Expr::SetError() { error_ = true; }

auto Expr::Type() const -> enum Type { return type_; }

auto Expr::ChildrenSize() const -> uint64_t {
  if (IsLiteral(type_)) {
    return 0;
  }
  if (IsUnary(type_)) {
    return 1;
  }
  if (IsBinary(type_)) {
    return 2;
  }

  assert(false);
  return 0;
}

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

void Literal::Description(std::string &out, uint64_t num) const {
  if (num == 0) {
    out += val_.ToString();
  }
}

[[nodiscard]] auto Literal::Complete() const -> bool { return true; }

PredicateLiteral::PredicateLiteral(Token val, Token left, Token right)
    : Literal(std::move(val)), left_var_(std::move(left)),
      right_var_(std::move(right)) {}

void PredicateLiteral::Description(std::string &out, uint64_t num) const {
  if (num == 0) {
    out += val_.ToString() + "(" + left_var_.ToString() + "," +
           right_var_.ToString() + ")";
  }
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

void UnaryExpr::Description(std::string &out, uint64_t num) const {
  if (num == 0) {
    out += "-";
  }
}

[[nodiscard]] auto UnaryExpr::Complete() const -> bool {
  return type_ != Type::kNull && expr_;
}

QuantifiedUnaryExpr::QuantifiedUnaryExpr(enum Type type, Token var)
    : UnaryExpr(type), var_(std::move(var)) {}

void QuantifiedUnaryExpr::Description(std::string &out, uint64_t num) const {
  if (num == 0) {
    if (type_ == Expr::Type::kExist) {
      out += "E";
    } else if (type_ == Expr::Type::kUniversal) {
      out += "A";
    }
    out += var_.ToString();
  }
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

void BinaryExpr::Description(std::string &out, uint64_t num) const {
  if (num == 0) {
    out += "(";
  } else if (num == 1) {
    if (type_ == Expr::Type::kAnd) {
      out += "^";
    } else if (type_ == Expr::Type::kOr) {
      out += "v";
    } else if (type_ == Expr::Type::kImpl) {
      out += ">";
    }
  } else if (num == 2) {
    out += ")";
  }
}