#include <cassert>
#include <deque>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "expr.hh"
#include "exprs/exprs.hh"
#include "formula.hh"
#include "tableau.hh"
#include "tokenizer.hh"

TableauFormula::TableauFormula(const Formula &formula) : Formula(formula) {}

TableauFormula::TableauFormula(std::shared_ptr<Expr> expr)
    : Formula(std::move(expr)) {}

[[nodiscard]] auto TableauFormula::Expand(ConstantManager &manager)
    -> std::vector<std::vector<TableauFormula>> {
  Token token;

  if (Type() == Expr::Type::kUniversal) {
    /*
      if Universal Formula => we need to get a constant based on
        - the number of constants used by the Formula
        - the number of available constants
    */
    std::optional requested_const = manager.GetConsts(const_num_);
    if (!requested_const.has_value()) {
      return {};
    }
    ++const_num_;
    token = std::move(requested_const.value());
  } else if (Type() == Expr::Type::kExist) {
    /*
      if Existential Formula => we need to add a constant to the list based on
        - the availability of the constant manager
    */
    if (!manager.CanAddConst()) {
      return {};
    }
    token = manager.AddConst();
  }

  /*
    Try to expand and Encapsulate all back to the TableauFormula

    This provides encapsulation and also ensures that the lifetime of the
    shared_ptr is properly managed
  */
  std::vector expansion = TableauFormula::Expand(expr_, token);
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

static auto Flatten(std::vector<Expr *> &flatten,
                    std::vector<uint64_t> &parents,
                    std::vector<std::vector<std::shared_ptr<Expr>>> &to_merge,
                    const std::shared_ptr<Expr> &expr, const Token &src)
    -> void {
  for (std::vector<std::vector<Expr>>::size_type i = 1; i < flatten.size();
       ++i) {
    if (!to_merge[i].empty()) {
      continue;
    }

    const auto children = flatten[i]->ViewChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      flatten.emplace_back(it->get());
      parents.emplace_back(i);
      to_merge.emplace_back();

      if (((*it)->Type() == Expr::Type::kExist ||
           (*it)->Type() == Expr::Type::kUniversal) &&
          (*it)->Infos()[0] == src) {
        // If var is bounded by new quantifier, store it to to_merge directly
        // we will then merge it by checking the same condition
        to_merge.back().emplace_back(expr);
      }
    }
  }
}

static auto Merge(const Token &src, std::vector<Expr *> &flatten,
                  std::vector<uint64_t> &parents,
                  std::vector<std::vector<std::shared_ptr<Expr>>> &to_merge,
                  const Token &dst) -> void {
  for (auto i = to_merge.size() - 1; i > 0; --i) {
    if (flatten[i]->Type() ==
        Expr::Type::kLiteral) { // Literal => constructs literal
      auto infos = flatten[i]->Infos();
      assert(infos.size() == 3);
      infos[1] = infos[1] == src ? dst : infos[1];
      infos[2] = infos[2] == src ? dst : infos[2];
      to_merge[parents[i]].emplace_back(std::make_shared<PredicateLiteral>(
          std::move(infos[0]), std::move(infos[1]), std::move(infos[2])));
    } else if (Expr::IsBinary(flatten[i]->Type())) { // Construct Binary
      to_merge[parents[i]].emplace_back(std::make_shared<BinaryExpr>(
          flatten[i]->Type(), std::move(to_merge[i][0]),
          std::move(to_merge[i][1])));
    } else if (flatten[i]->Type() == Expr::Type::kExist ||
               flatten[i]->Type() ==
                   Expr::Type::kUniversal) { // Quantified Unary
      if (flatten[i]->Infos()[0] ==
          src) { // if variable is re-bounded => DO a simple copy
        to_merge[parents[i]].emplace_back(std::move(to_merge[i][0]));
      } else { // otherwise => Construct Quantified Expr
        to_merge[parents[i]].emplace_back(std::make_shared<QuantifiedUnaryExpr>(
            flatten[i]->Type(), std::move(flatten[i]->Infos()[0]),
            std::move(to_merge[i][0])));
      }
    } else if (flatten[i]->Type() ==
               Expr::Type::kNeg) { // Negation => Construct UnaryExpr
      to_merge[parents[i]].emplace_back(std::make_shared<UnaryExpr>(
          flatten[i]->Type(), std::move(to_merge[i][0])));
    } else { // Unreachable
      assert(false);
    }
  }
}

static auto CopyAndReplace(const Token &src, const std::shared_ptr<Expr> &expr,
                           const Token &dst) -> std::shared_ptr<Expr> {
  std::vector<Expr *> flatten{{}, expr.get()};
  std::vector<uint64_t> parents{0, 0};
  std::vector<std::vector<std::shared_ptr<Expr>>> to_merge{{}, {}};

  if ((expr->Type() == Expr::Type::kExist ||
       expr->Type() == Expr::Type::kUniversal) &&
      expr->Infos()[0] == src) {
    // If var is bounded by new quantifier, store it to to_merge directly
    // we will then merge it by checking the same condition
    to_merge[1].emplace_back(expr);
  }

  // Flatten the AST
  Flatten(flatten, parents, to_merge, expr, src);

  assert(to_merge.size() == flatten.size());

  // Merge back all the changes
  Merge(src, flatten, parents, to_merge, dst);

  return std::move(to_merge[0][0]);
}

[[nodiscard]] auto TableauFormula::Expand(const std::shared_ptr<Expr> &expr,
                                          const Token &token)
    -> std::vector<std::vector<std::shared_ptr<Expr>>> {
  (void)token;
  if (expr->Type() == Expr::Type::kAnd) { // Alpha expansion
    return {expr->ViewChildren()};
  }
  if (expr->Type() == Expr::Type::kOr) { // Beta expansion
    return {{expr->ViewChildren()[0]}, {expr->ViewChildren()[1]}};
  }
  if (expr->Type() == Expr::Type::kImpl) { // Beta expansion
    return {{std::make_shared<UnaryExpr>(Expr::Type::kNeg,
                                         expr->ViewChildren()[0])},
            {expr->ViewChildren()[1]}};
  }
  if (expr->Type() == Expr::Type::kExist ||
      expr->Type() == Expr::Type::kUniversal) {
    // Delta/Gamma expansion
    /*
      1. Copy every node that needs to be copied
        - Every node needs to be copied except the node where the variable is
          quantified again
        - They need to be copied because we need to replace the Token with new
          constants but we don't want to pollute the original Formula
      2. After copying, we return the formulas based on the rule of Delta
         expansion
        - Gamma will be added back in TryExpand
    */
    return {{CopyAndReplace(expr->Infos()[0], expr->ViewChildren()[0], token)}};
  }
  if (expr->Type() == Expr::Type::kNeg) {
    std::shared_ptr<Expr> children = expr->ViewChildren()[0];

    // If we are negating literal, we return Neg+Literal
    // by the definition of literals in Tableau
    if (children->Type() == Expr::Type::kLiteral) {
      return {{std::make_shared<UnaryExpr>(Expr::Type::kNeg,
                                           expr->ViewChildren()[0])}};
    }

    // If Unary
    if (Expr::IsUnary(children->Type())) {
      // If Neg, we skip the double Negation
      if (children->Type() == Expr::Type::kNeg) {
        return {{std::move(children->ViewChildren()[0])}};
      }
      // Otherwise, we negate the Quantified Formula based on their rule
      if (children->Type() == Expr::Type::kExist ||
          children->Type() == Expr::Type::kUniversal) {
        std::vector infos = children->Infos();
        assert(infos.size() == 1);

        return {{std::make_shared<QuantifiedUnaryExpr>(
            Expr::Negate(children->Type()), std::move(infos[0]),
            std::make_shared<UnaryExpr>(
                Expr::Type::kNeg, std::move(children->ViewChildren()[0])))}};
      }
    }

    // If Binary => we negate them based on their rules
    if (children->Type() == Expr::Type::kAnd ||
        children->Type() == Expr::Type::kOr) {
      std::vector children_of_children = children->ViewChildren();
      auto neg_children_left = std::make_shared<UnaryExpr>(
          Expr::Type::kNeg, std::move(children_of_children[0]));
      auto neg_children_right = std::make_shared<UnaryExpr>(
          Expr::Type::kNeg, std::move(children_of_children[1]));
      std::shared_ptr<Expr> node = std::make_shared<BinaryExpr>(
          Expr::Negate(children->Type()), std::move(neg_children_left),
          std::move(neg_children_right));
      return {{std::move(node)}};
    }

    // If implication => Negate it based on its fule
    if (children->Type() == Expr::Type::kImpl) {
      std::vector children_of_children = children->ViewChildren();
      auto neg_children_right = std::make_shared<UnaryExpr>(
          Expr::Type::kNeg, std::move(children_of_children[1]));
      std::shared_ptr<Expr> impl_node = std::make_shared<BinaryExpr>(
          Expr::Negate(children->Type()), std::move(children_of_children[0]),
          std::move(neg_children_right));
      return {{std::move(impl_node)}};
    }
  }

  assert(false);
  return {};
}

Theory::Theory(const TableauFormula &formula) { Append(formula); }

auto Theory::Undecidable() const -> bool { return undecidable_; }

auto Theory::Close() const -> bool { return close_; }

// An encapsulation of Append
// Help us to filter out literal and Assign formula to their appropriate
// structure
auto Theory::Append(const TableauFormula &formula) -> void {
  if (formula.Type() ==
      Expr::Type::kLiteral) { // if tableau literal => to literal or neg_literal
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

  formulas_.emplace(formula); // otherwise, go to priority_queue
}

auto Theory::TryExpand() -> std::vector<Theory> {
  // only Quantified Formula can yield empty
  while (!formulas_.empty()) {
    auto formula = formulas_.top();
    formulas_.pop();

    // Try expanding the formula, if we cannot expand
    //  - no more constants
    //  - reach constant limits
    // we continue to try next one, and drop the current one
    //
    // We can safely drop even in the second condition because
    // in our priority_queue, existential quantifier always come before
    // universal
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

  /*
    After consuming all the formulas, there are only two possibilities:
      - we could not expand because no more constant is available (but we could
        have more)
      - we reach the constant limit

    In the second case, we need to mark our theory as undecidable
  */
  if (formulas_.empty() && !manager_.CanAddConst()) {
    // we cannot decide its satisfiability
    undecidable_ = true;
  }

  return {};
}

auto Tableau::Solve(const Parser::ParserOutput &parser_out) -> TableauResult {
  std::deque<Theory> queue;
  queue.emplace_back(TableauFormula{parser_out.Formula()});

  bool undecidable{
      false}; // to mark whether we have encountered undecidable formula

  while (!queue.empty()) {
    Theory theory{std::move(queue.front())};
    queue.pop_front();

    std::vector theories = theory.TryExpand();

    if (theory.Undecidable()) {
      undecidable = true;
      continue;
    }

    // If theory is not undecidable and is not closed
    // and we could not generate more theories from it
    // we know it's satisfiable
    //
    // it's not closed because we guarantee any formula added
    // to the queue is not closed, and TryExpand will not yield
    // close state.
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