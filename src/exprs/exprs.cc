#include <cassert>

#include "exprs/expr.hh"

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
