#pragma once

#include <memory>
#include <vector>

#include "exprs/binary.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"

class InfoVisitor {
public:
  auto operator()(const Literal &literal) -> void {
    infos_.push_back(literal.val_);
  }

  auto operator()(const PredicateLiteral &predicate) -> void {
    infos_.push_back(predicate.val_);
    infos_.push_back(predicate.left_var_);
    infos_.push_back(predicate.right_var_);
  }

  auto operator()(const UnaryExpr &unary) -> void { (void)unary; }

  auto operator()(const QuantifiedUnaryExpr &quantified) -> void {
    infos_.push_back(quantified.var_);
  }

  auto operator()(const BinaryExpr &binary) -> void { (void)binary; }

  auto Infos() -> std::vector<Token> & { return infos_; }

private:
  std::vector<Token> infos_{};
};
