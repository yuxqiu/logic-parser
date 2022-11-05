#pragma once

#include <cstddef>
#include <memory>
#include <stack>
#include <vector>

#include "expr.hh"
#include "formula.hh"

class Formula {
public:
  explicit Formula() = default;
  explicit Formula(std::shared_ptr<Expr> expr);
  ~Formula();

  Formula(const Formula &) = default;
  auto operator=(const Formula &) -> Formula & = delete;

  Formula(Formula &&owner) noexcept = default;
  auto operator=(Formula &&) noexcept -> Formula & = delete;

  [[nodiscard]] auto Type() const -> enum Expr::Type;

  // Description of the entire formula
  [[nodiscard]] auto Description() const -> std::string;

  [[nodiscard]] auto ViewChildren() const -> std::vector<Formula>;

protected:
  static void ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                         std::string &out, Expr *expr);
  void ReleaseResources();

  std::shared_ptr<Expr> expr_;
};