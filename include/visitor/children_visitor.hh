#pragma once

#include <memory>
#include <vector>

#include "exprs/binary.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

#include "expr_visitor.hh"

class ChildrenVisitor final : public ExprVisitor {
public:
  auto Visit(const Literal &literal) -> void final { (void)literal; }

  auto Visit(const PredicateLiteral &predicate) -> void final {
    (void)predicate;
  }

  auto Visit(const UnaryExpr &unary) -> void final {
    children_.push_back(unary.expr_);
  }

  auto Visit(const QuantifiedUnaryExpr &quantified) -> void final {
    children_.push_back(quantified.expr_);
  }

  auto Visit(const BinaryExpr &binary) -> void final {
    children_.push_back(binary.expr_lhs_);
    children_.push_back(binary.expr_rhs_);
  }

  auto ChildrenSize() -> std::vector<std::shared_ptr<Expr>>::size_type {
    return children_.size();
  }

  auto ViewChildren() -> std::vector<std::shared_ptr<Expr>> & {
    return children_;
  }

private:
  std::vector<std::shared_ptr<Expr>> children_{};
};
