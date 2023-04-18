#pragma once

#include <memory>
#include <string>
#include <vector>

#include "constant.hh"
#include "exprs/kind.hh"
#include "tokenizer.hh"
#include "visitor/expr_visitor.hh"

class Expr {
public:
  [[nodiscard]] auto Type() const -> ExprKind { return type_; }
  auto SetType(ExprKind type) -> void { type_ = type; }

  // Get/Set error during constructing stage
  [[nodiscard]] auto Error() const -> bool { return error_; }
  auto SetError() -> void { error_ = true; }

  explicit Expr() = default;
  explicit Expr(ExprKind type) : type_(type) {}

  virtual ~Expr() = default;
  Expr(const Expr &&) = delete;
  Expr(Expr &&) = delete;
  auto operator=(const Expr &) -> Expr & = delete;
  auto operator=(Expr &&) -> Expr & = delete;

  // Append determines which inserted order is correct
  // when we try to construct the expr from raw string
  virtual auto Append(std::shared_ptr<Expr> expr) -> void = 0;
  virtual auto Append(ExprKind type) -> void = 0;

  // Whether of not the Expr is completely built
  [[nodiscard]] virtual auto Complete() const -> bool = 0;

  virtual auto Accept(ExprVisitor &visitor) const -> void = 0;

private:
  ExprKind type_{ExprKind::kNull};
  bool error_{false};

  friend class Formula;
};
