#include "tableau.hh"
#include <deque>
#include <queue>
#include <utility>
#include <vector>

#include "expr.hh"

Theory::Theory(const Formula &formula) { Append(formula); }

auto Theory::Undecidable() const -> bool { return undecidable_; }

auto Theory::Close() const -> bool { return close_; }

void Theory::Append(const Formula &formula) {
  if (formula.Type() == Expr::Type::kLiteral) {
    auto literal = formula.Infos()[0];
    if (neg_literals_.find(literal) != neg_literals_.end()) {
      close_ = true;
    } else {
      literals_.emplace(std::move(literal));
    }
    return;
  }

  if (formula.Type() == Expr::Type::kNeg &&
      formula.ViewChildren()[0].Type() == Expr::Type::kLiteral) {
    auto literal = formula.ViewChildren()[0].Infos()[0];
    if (literals_.find(literal) != literals_.end()) {
      close_ = true;
    } else {
      neg_literals_.emplace(std::move(literal));
    }
    return;
  }

  formulas_.emplace(formula);
}

auto Theory::TryExpand() -> std::vector<Theory> {
  // only Quantified Formula can yield empty
  while (!formulas_.empty() && formulas_.top().Expand().empty()) {
    formulas_.pop();
  }

  // If empty
  if (formulas_.empty()) {
    // and const_num is greater than 10,
    // we cannot decide its satisfiability
    if (!manager_.CanAddConst()) {
      undecidable_ = true;
    }
    return {};
  }

  auto formula = formulas_.top();
  formulas_.pop();
  std::vector expansions = formula.Expand();
  std::vector<Theory> new_theories;
  new_theories.reserve(expansions.size());

  for (auto &expansion : expansions) {
    Theory new_theory = *this;
    for (const Formula &new_formula : expansion) {
      new_theory.Append(new_formula);
    }
    if (formula.Type() == Expr::Type::kUniversal) {
      new_theory.Append(formula);
    }
    new_theories.emplace_back(std::move(new_theory));
  }

  return new_theories;
}

auto Tableau::Solve(const Parser::ParserOutput &parser_out) -> TableauResult {
  std::deque<Theory> queue;
  queue.emplace_back(parser_out.Formula());

  bool undecidable{false};

  while (!queue.empty()) {
    Theory theory{std::move(queue.front())};
    queue.pop_front();

    std::vector theories = theory.TryExpand();

    if (theory.Undecidable()) {
      undecidable = true;
      continue;
    }

    if (theories.empty()) {
      return TableauResult::kSatisfiable;
    }

    for (auto &new_theory : theories) {
      if (!new_theory.Close()) {
        queue.emplace_back(std::move(new_theory));
      }
    }
  }

  return undecidable ? TableauResult::kUndecidable
                     : TableauResult::kUnsatisfiable;
}