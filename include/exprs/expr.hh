#pragma once

#include <variant>

#include "exprs/binary.hh"
#include "exprs/kind.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

using ExprInternal = std::variant<Literal, PredicateLiteral, UnaryExpr,
                                  QuantifiedUnaryExpr, BinaryExpr>;

class Expr : public ExprInternal {
public:
  //   use variant constructor
  using ExprInternal::ExprInternal;

  [[nodiscard]] auto Type() const -> ExprKind {
    return std::visit([](const auto &expr) { return expr.Type(); }, *this);
  }

  [[nodiscard]] auto Error() const -> bool {
    return std::visit([](const auto &expr) { return expr.error_; }, *this);
  }

  auto Append(const std::shared_ptr<Expr> &expr_append) -> void {
    std::visit([&expr_append](auto &expr) { expr.Append(expr_append); }, *this);
  }

  auto Append(ExprKind type) -> void {
    std::visit([type](auto &expr) { expr.Append(type); }, *this);
  }

  [[nodiscard]] auto Complete() -> bool {
    return std::visit([](auto &expr) { return expr.Complete(); }, *this);
  }
};
