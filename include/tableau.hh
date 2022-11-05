#pragma once

#include <cstddef>
#include <queue>
#include <unordered_set>

#include "formula.hh"
#include "parser.hh"
#include "constant.hh"

class Theory {
public:
  [[nodiscard]] auto Undecidable() const -> bool;
  [[nodiscard]] auto Close() const -> bool;

  explicit Theory(const Formula &formula);

  // expandable => non-empty vector
  // Un-expandable => empty vector
  auto TryExpand() -> std::vector<Theory>;

  void Append(const Formula &formula);

private:
  std::priority_queue<Formula> formulas_{};
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