#pragma once

#include <cstddef>
#include <memory>
#include <stack>
#include <vector>

#include "expr.hh"
#include "formula.hh"

/*
  Manage the lifetime of the Expr
*/
class Formula {
public:
  // We could safely rely on default copy/move constructor/assignment as
  // the destructor follows the semantics of ref_count
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

private:
  static auto ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                         std::string &out, Expr *expr) -> void;
};