#pragma once

#include <memory>
#include <vector>

#include "exprs/binary.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

#include "expr_visitor.hh"

class InfoVisitor final : public ExprVisitor {
public:
  auto Visit(const Literal &literal) -> void final {
    infos_.emplace_back(literal.val_);
  }

  auto Visit(const PredicateLiteral &predicate) -> void final {
    infos_.emplace_back(predicate.val_);
    infos_.emplace_back(predicate.left_var_);
    infos_.emplace_back(predicate.right_var_);
  }

  auto Visit(const UnaryExpr &unary) -> void final { (void)unary; }

  auto Visit(const QuantifiedUnaryExpr &quantified) -> void final {
    infos_.emplace_back(quantified.var_);
  }

  auto Visit(const BinaryExpr &binary) -> void final { (void)binary; }

  auto Infos() -> std::vector<Token> & { return infos_; }

private:
  std::vector<Token> infos_{};
};