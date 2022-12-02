#pragma once

#include "expr.hh"
#include "expr_visitor.hh"

#include <memory>
#include <vector>

class ChildrenVisitor : public ExprVisitor {
public:
  auto Visit(const Literal &literal) -> void final { (void)literal; }

  auto Visit(const PredicateLiteral &predicate) -> void final {
    (void)predicate;
  }

  auto Visit(const UnaryExpr &unary) -> void final {
    children_.emplace_back(unary.expr_);
  }

  auto Visit(const QuantifiedUnaryExpr &quantified) -> void final {
    children_.emplace_back(quantified.expr_);
  }

  auto Visit(const BinaryExpr &binary) -> void final {
    children_.emplace_back(binary.expr_lhs_);
    children_.emplace_back(binary.expr_rhs_);
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