#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "exprs/expr.hh"
#include "formula.hh"

/*
  Manage the lifetime of the Expr
*/
class Formula {
public:
  explicit Formula() = default;
  explicit Formula(std::shared_ptr<Expr> expr) : expr_{std::move(expr)} {}

  ~Formula();
  Formula(const Formula &) = default;
  Formula(Formula &&) = default;
  auto operator=(const Formula &) -> Formula & = default;
  auto operator=(Formula &&) -> Formula & = default;

  [[nodiscard]] auto Type() const -> enum Expr::Type { return expr_->Type(); }

  [[nodiscard]] auto Description() const -> std::string;

  [[nodiscard]] auto Connective() const -> std::string;

  [[nodiscard]] auto ViewChildren() const -> std::vector<Formula>;

protected:
  std::shared_ptr<Expr> expr_{};
};
