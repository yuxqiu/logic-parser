#pragma once

#include <cstddef>
#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>

#include "constant.hh"
#include "expr.hh"
#include "formula.hh"
#include "parser.hh"

class TableauFormula : public Formula {
public:
  explicit TableauFormula(const Formula &formula);

  [[nodiscard]] auto Expand(ConstantManager &manager)
      -> std::vector<std::vector<TableauFormula>>;

private:
  // next needed constant num
  uint64_t const_num_{0};

  // 1. Number of branches
  // 2. Number of children
  [[nodiscard]] static auto Expand(const std::shared_ptr<Expr> &expr,
                                   const Token &token)
      -> std::vector<std::vector<std::shared_ptr<Expr>>>;
};

class Theory {
public:
  [[nodiscard]] auto Undecidable() const -> bool;
  [[nodiscard]] auto Close() const -> bool;

  explicit Theory(const TableauFormula &formula);

  // expandable => non-empty vector
  // Un-expandable => empty vector
  auto TryExpand() -> std::vector<Theory>;

  auto Append(const TableauFormula &formula) -> void;

private:
  std::priority_queue<TableauFormula, std::vector<TableauFormula>,
                      std::greater<>>
      formulas_{};
  std::unordered_set<Token> literals_{};
  std::unordered_set<Token> neg_literals_{};

  ConstantManager manager_{};
  bool undecidable_{false};
  bool close_{false};
};

class Tableau {
public:
  enum class TableauResult { kUnsatisfiable, kSatisfiable, kUndecidable };

  [[nodiscard]] static auto Solve(const Parser::ParserOutput &parser_out)
      -> TableauResult;
};