#pragma once

#include <cstddef>
#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>

#include "constant.hh"
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

  friend auto operator>(const TableauFormula &lhs, const TableauFormula &rhs)
      -> bool;
};

class Theory {
public:
  [[nodiscard]] auto Undecidable() const -> bool { return undecidable_; }
  [[nodiscard]] auto Close() const -> bool { return close_; }

  explicit Theory(const TableauFormula &formula) { Append(formula); }

  // Expandable => non-empty vector
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