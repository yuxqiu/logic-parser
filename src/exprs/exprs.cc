#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "constant.hh"
#include "exprs/binary.hh"
#include "exprs/exprs.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

auto Expr::Negate(enum Expr::Type type) -> enum Expr::Type {
  switch (type){
    case Type::kAnd : return Type::kOr;
    case Type::kExist : return Type::kUniversal;
    case Type::kOr : [[fallthrough]];
    case Type::kImpl : return Type::kAnd;
    case Type::kUniversal : return Type::kExist;
    case Type::kNull : case Type::kLiteral : case Type::kNeg : break;
  }

assert(false);
return Type::kNull;
}

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
  case Type::kNull:
  case Type::kLiteral:
    break;
  }

  assert(false);
  return "Unreachable";
}

auto static LiteralDescription(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (num == 0) {
    auto infos = expr->Infos();
    out += infos[0].ToString();
    if (infos.size() == 3) {
      out += "(" + infos[1].ToString() + "," + infos[2].ToString() + ")";
    }
  }
}

auto static UnaryDescription(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (num == 0) {
    out += Expr::TypeToString(expr->Type());
    if (expr->Type() == Expr::Type::kExist ||
        expr->Type() == Expr::Type::kUniversal) {
      out += expr->Infos()[0].ToString();
    }
  }
}

auto static BinaryDescription(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (num == 0) {
    out += "(";
  } else if (num == 1) {
    out += Expr::TypeToString(expr->Type());
  } else if (num == 2) {
    out += ")";
  }
}

auto Expr::Description(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (IsBinary(expr->Type())) {
    BinaryDescription(expr, out, num);
  } else if (IsUnary(expr->Type())) {
    UnaryDescription(expr, out, num);
  } else if (IsLiteral(expr->Type())) {
    LiteralDescription(expr, out, num);
  }
}