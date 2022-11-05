#include "exprs/exprs.hh"
#include "exprs/binary.hh"
#include "exprs/unary.hh"
#include <cassert>
#include <memory>
#include <string>

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

[[nodiscard]] auto Expr::Expand(const std::shared_ptr<Expr> &expr)
    -> std::vector<std::vector<std::shared_ptr<Expr>>> {
  if (expr->Type() == Expr::Type::kAnd) {
    return {expr->ViewChildren()};
  }
  if (expr->Type() == Expr::Type::kOr) {
    return {{expr->ViewChildren()[0]}, {expr->ViewChildren()[1]}};
  }
  if (expr->Type() == Expr::Type::kImpl) {
    return {{std::make_shared<UnaryExpr>(Type::kNeg, expr->ViewChildren()[0])},
            {expr->ViewChildren()[1]}};
  }
  if (expr->Type() == Expr::Type::kExist) {
    return {};
  }
  if (expr->Type() == Expr::Type::kUniversal) {
    return {};
  }
  if (expr->Type() == Type::kNeg) {
    std::shared_ptr<Expr> children = expr->ViewChildren()[0];

    if (children->Type() == Expr::Type::kLiteral) {
      return {
          {std::make_shared<UnaryExpr>(Type::kNeg, expr->ViewChildren()[0])}};
    }

    if (IsUnary(children->Type())) {
      if (children->Type() == Type::kNeg) {
        return {{std::move(children->ViewChildren()[0])}};
      }
      if (children->Type() == Expr::Type::kExist ||
          children->Type() == Expr::Type::kUniversal) {
        std::vector infos = children->Infos();
        assert(infos.size() == 1);

        return {{std::make_shared<QuantifiedUnaryExpr>(
            Negate(children->Type()), std::move(infos[0]),
            std::make_shared<UnaryExpr>(
                Type::kNeg, std::move(children->ViewChildren()[0])))}};
      }
    }

    if (children->Type() == Expr::Type::kAnd ||
        children->Type() == Expr::Type::kOr) {
      std::vector children_of_children = children->ViewChildren();
      auto neg_children_left = std::make_shared<UnaryExpr>(
          Type::kNeg, std::move(children_of_children[0]));
      auto neg_children_right = std::make_shared<UnaryExpr>(
          Type::kNeg, std::move(children_of_children[1]));
      std::shared_ptr<Expr> node = std::make_shared<BinaryExpr>(
          Negate(children->Type()), std::move(neg_children_left),
          std::move(neg_children_right));
      return {{std::move(node)}};
    }

    if (children->Type() == Expr::Type::kImpl) {
      std::vector children_of_children = children->ViewChildren();
      auto neg_children_right = std::make_shared<UnaryExpr>(
          Type::kNeg, std::move(children_of_children[1]));
      std::shared_ptr<Expr> impl_node = std::make_shared<BinaryExpr>(
          Negate(children->Type()), std::move(children_of_children[0]),
          std::move(neg_children_right));
      return {{std::move(impl_node)}};
    }
  }

  return {};
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
  if (expr->Type() == Expr::Type::kExist ||
      expr->Type() == Expr::Type::kUniversal) {
    if (num == 0) {
      if (expr->Type() == Expr::Type::kExist) {
        out += "E";
      } else if (expr->Type() == Expr::Type::kUniversal) {
        out += "A";
      }
      out += expr->Infos()[0].ToString();
    }
  } else if (expr->Type() == Expr::Type::kNeg) {
    if (num == 0) {
      out += "-";
    }
  }
}

void static BinaryDescription(const Expr *expr, std::string &out,
                              uint64_t num) {
  if (num == 0) {
    out += "(";
  } else if (num == 1) {
    if (expr->Type() == Expr::Type::kAnd) {
      out += "^";
    } else if (expr->Type() == Expr::Type::kOr) {
      out += "v";
    } else if (expr->Type() == Expr::Type::kImpl) {
      out += ">";
    }
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