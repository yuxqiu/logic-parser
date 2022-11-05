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

  [[nodiscard]] auto Type() const -> enum Expr::Type;

  [[nodiscard]] auto Description() const -> std::string;
  [[nodiscard]] auto Infos() const -> std::vector<Token>;

  [[nodiscard]] auto ViewChildren() const -> std::vector<Formula>;

  [[nodiscard]] auto Expand() const -> std::vector<std::vector<Formula>>;

private:
  static void ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                         std::string &out, Expr *expr);
  void ReleaseResources();

  std::shared_ptr<Expr> expr_;

  friend auto operator<(const Formula &lhs, const Formula &rhs) -> bool;
};

auto operator<(const Formula &lhs, const Formula &rhs) -> bool;