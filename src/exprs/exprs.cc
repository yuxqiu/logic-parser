#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "constant.hh"
#include "exprs/binary.hh"
#include "exprs/exprs.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

Expr::Expr(enum Type type) : type_(type) {}

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

auto Expr::Negate(enum Expr::Type type) -> enum Expr::Type {
  switch (type){
    case Type::kAnd : return Type::kOr;
    case Type::kExist : return Type::kUniversal;
    case Type::kOr : [[fallthrough]];
    case Type::kImpl : return Type::kAnd;
    case Type::kUniversal : return Type::kExist;
    default : break;
  }

assert(false);
return Type::kNull;
}

auto operator<(const Expr &lhs, const Expr &rhs) -> bool {
  return lhs.type_ < rhs.type_;
}

auto Expr::TypeToString(enum Expr::Type type) -> std::string {
  switch (type) {
  case Expr::Type::kAnd:
    return "^";
  case Expr::Type::kOr:
    return "v";
  case Expr::Type::kImpl:
    return ">";
  case Expr::Type::kNeg:
    return "-";
  case Expr::Type::kExist:
    return "E";
  case Expr::Type::kUniversal:
    return "A";
  default:
    assert(false);
  }

  return "Unreachable";
}

void static LiteralDescription(const Expr *expr, std::string &out,
                               uint64_t num) {
  if (num == 0) {
    auto infos = expr->Infos();
    out += infos[0].ToString();
    if (infos.size() == 3) {
      out += "(" + infos[1].ToString() + "," + infos[2].ToString() + ")";
    }
  }
}

void static UnaryDescription(const Expr *expr, std::string &out, uint64_t num) {
  if (num == 0) {
    out += Expr::TypeToString(expr->Type());
    if (expr->Type() == Expr::Type::kExist ||
        expr->Type() == Expr::Type::kUniversal) {
      out += expr->Infos()[0].ToString();
    }
  }
}

void static BinaryDescription(const Expr *expr, std::string &out,
                              uint64_t num) {
  if (num == 0) {
    out += "(";
  } else if (num == 1) {
    out += Expr::TypeToString(expr->Type());
  } else if (num == 2) {
    out += ")";
  }
}

void Expr::Description(const Expr *expr, std::string &out, uint64_t num) {
  if (IsBinary(expr->Type())) {
    BinaryDescription(expr, out, num);
  } else if (IsUnary(expr->Type())) {
    UnaryDescription(expr, out, num);
  } else if (IsLiteral(expr->Type())) {
    LiteralDescription(expr, out, num);
  }
}