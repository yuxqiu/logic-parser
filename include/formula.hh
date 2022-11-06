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
  Formula(const Formula &) = default;

  explicit Formula(std::shared_ptr<Expr> expr);
  ~Formula();

  [[nodiscard]] auto Type() const -> enum Expr::Type;

  [[nodiscard]] auto Description() const -> std::string;
  [[nodiscard]] auto Infos() const -> std::vector<Token>;

  [[nodiscard]] auto ViewChildren() const -> std::vector<Formula>;

protected:
  std::shared_ptr<Expr> expr_;

private:
  static void ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                         std::string &out, Expr *expr);
  void ReleaseResources();

  friend auto operator>(const Formula &lhs, const Formula &rhs) -> bool;
};

auto operator>(const Formula &lhs, const Formula &rhs) -> bool;