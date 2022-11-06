#include <deque>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "expr.hh"
#include "formula.hh"
#include "tableau.hh"
#include "tokenizer.hh"

TableauFormula::TableauFormula(const Formula &formula) : Formula(formula) {}

TableauFormula::TableauFormula(std::shared_ptr<Expr> expr)
    : Formula(std::move(expr)) {}

[[nodiscard]] auto TableauFormula::Expand(ConstantManager &manager)
    -> std::vector<std::vector<TableauFormula>> {
  Token token;

  // Decorator of Quantified Expr
  if (Type() == Expr::Type::kUniversal) {
    std::optional requested_const = manager.GetConsts(const_num_);
    if (!requested_const.has_value()) {
      return {};
    }
    ++const_num_;
    token = std::move(requested_const.value());
  } else if (Type() == Expr::Type::kExist) {
    if (!manager.CanAddConst()) {
      return {};
    }
    token = manager.AddConst();
  }

  std::vector expansion = Expr::Expand(expr_, token);
  std::vector<std::vector<TableauFormula>> ret;
  ret.reserve(expansion.size());

  for (auto &one_expansion : expansion) {
    std::vector<TableauFormula> formulas;
    formulas.reserve(one_expansion.size());
    for (auto &new_formula : one_expansion) {
      formulas.emplace_back(std::move(new_formula));
    }
    ret.emplace_back(std::move(formulas));
  }

  return ret;
}

Theory::Theory(const TableauFormula &formula) { Append(formula); }

auto Theory::Undecidable() const -> bool { return undecidable_; }

auto Theory::Close() const -> bool { return close_; }

void Theory::Append(const TableauFormula &formula) {
  if (formula.Type() == Expr::Type::kLiteral) {
    // use description to deal with prop literal and pred literal
    auto literal = formula.Description();
    if (neg_literals_.find(Token{literal}) != neg_literals_.end()) {
      close_ = true;
    } else {
      literals_.emplace(std::move(literal));
    }
    return;
  }

  if (formula.Type() == Expr::Type::kNeg &&
      formula.ViewChildren()[0].Type() == Expr::Type::kLiteral) {
    auto literal = formula.ViewChildren()[0].Description();
    if (literals_.find(Token{literal}) != literals_.end()) {
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
  while (!formulas_.empty()) {
    auto formula = formulas_.top();
    formulas_.pop();
    auto expansions = formula.Expand(manager_);
    if (expansions.empty()) {
      continue;
    }

    std::vector<Theory> new_theories;
    new_theories.reserve(expansions.size());

    for (auto &expansion : expansions) {
      Theory new_theory = *this;
      for (const auto &new_formula : expansion) {
        new_theory.Append(new_formula);
      }

      // Gamma formula needs to be added back to the Theory
      if (formula.Type() == Expr::Type::kUniversal) {
        new_theory.Append(formula);
      }

      new_theories.emplace_back(std::move(new_theory));
    }

    return new_theories;
  }

  // If empty and const_num is greater than 10,
  if (formulas_.empty() && !manager_.CanAddConst()) {
    // we cannot decide its satisfiability
    undecidable_ = true;
  }

  return {};
}

auto Tableau::Solve(const Parser::ParserOutput &parser_out) -> TableauResult {
  std::deque<Theory> queue;
  queue.emplace_back(TableauFormula{parser_out.Formula()});

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