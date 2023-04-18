#pragma once

#include <memory>
#include <vector>

#include "exprs/binary.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

class ChildrenVisitor {
public:
  auto operator()(const Literal &literal) -> void { (void)literal; }

  auto operator()(const PredicateLiteral &predicate) -> void {
    (void)predicate;
  }

  auto operator()(const UnaryExpr &unary) -> void {
    children_.push_back(unary.expr_);
  }

  auto operator()(const QuantifiedUnaryExpr &quantified) -> void {
    children_.push_back(quantified.expr_);
  }

  auto operator()(const BinaryExpr &binary) -> void {
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
