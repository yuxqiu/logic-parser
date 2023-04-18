#include <cassert>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "exprs/binary.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"
#include "formula.hh"
#include "tableau.hh"
#include "tokenizer.hh"
#include "utils/helper.hh"
#include "visitor/children_visitor.hh"
#include "visitor/info_visitor.hh"

TableauFormula::TableauFormula(const Formula &formula) : Formula(formula) {}

namespace {
auto Flatten(std::vector<Expr *> &flatten, std::vector<uint64_t> &parents,
             std::vector<std::vector<std::shared_ptr<Expr>>> &to_merge,
             const Token &src) -> void {
  for (std::vector<std::vector<Expr>>::size_type i = 1; i < flatten.size();
       ++i) {
    if (!to_merge[i].empty()) {
      continue;
    }

    ChildrenVisitor children_visitor;
    std::visit(children_visitor, *flatten[i]);

    const auto &children = children_visitor.ViewChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      flatten.push_back(it->get());
      parents.push_back(i);
      to_merge.emplace_back();

      InfoVisitor info_visitor;
      std::visit(info_visitor, **it);

      const auto type = (*it)->Type();

      if ((type == ExprKind::kExist || type == ExprKind::kUniversal) &&
          info_visitor.Infos()[0] == src) {
        // If var is bounded by new quantifier, store it to to_merge directly
        // we will then merge it by checking the same condition
        to_merge.back().push_back(*it);
      }
    }
  }
}

auto Merge(const Token &src, std::vector<Expr *> &flatten,
           std::vector<uint64_t> &parents,
           std::vector<std::vector<std::shared_ptr<Expr>>> &to_merge,
           const Token &dst) -> void {
  for (auto i = to_merge.size() - 1; i > 0; --i) {
    const auto flattened_type = flatten[i]->Type();

    if (ExprKind::IsLiteral(flattened_type)) { // Literal => constructs literal
      InfoVisitor info_visitor;
      std::visit(info_visitor, *flatten[i]);

      auto &infos = info_visitor.Infos();
      assert(infos.size() == 3);
      infos[1] = infos[1] == src ? dst : infos[1];
      infos[2] = infos[2] == src ? dst : infos[2];
      to_merge[parents[i]].push_back(std::make_shared<Expr>(
          std::in_place_type_t<PredicateLiteral>{}, std::move(infos[0]),
          std::move(infos[1]), std::move(infos[2])));
      continue;
    }

    if (ExprKind::IsBinary(flattened_type)) { // Construct Binary
      to_merge[parents[i]].push_back(std::make_shared<Expr>(
          std::in_place_type_t<BinaryExpr>{}, flattened_type,
          std::move(to_merge[i][0]), std::move(to_merge[i][1])));
      continue;
    }

    if (flattened_type == ExprKind::kExist ||
        flattened_type == ExprKind::kUniversal) { // Quantified Unary
      InfoVisitor info_visitor;
      std::visit(info_visitor, *flatten[i]);

      auto &infos = info_visitor.Infos();

      if (infos[0] == src) { // if variable is re-bounded => DO a simple copy
        to_merge[parents[i]].push_back(std::move(to_merge[i][0]));
      } else { // otherwise => Construct Quantified Expr
        to_merge[parents[i]].push_back(std::make_shared<Expr>(
            std::in_place_type_t<QuantifiedUnaryExpr>{}, flattened_type,
            std::move(infos[0]), std::move(to_merge[i][0])));
      }
      continue;
    }

    if (flattened_type == ExprKind::kNeg) { // Negation => Construct UnaryExpr
      to_merge[parents[i]].push_back(
          std::make_shared<Expr>(std::in_place_type_t<UnaryExpr>{},
                                 flattened_type, std::move(to_merge[i][0])));
      continue;
    }

    unreachable();
  }
}

auto CopyAndReplace(const Token &src, std::shared_ptr<Expr> expr,
                    const Token &dst) -> std::shared_ptr<Expr> {
  // Potential Optimization Here
  // Encapsulate them inside a struct is better for locality
  std::vector<Expr *> flatten{{}, expr.get()};
  std::vector<uint64_t> parents{0, 0};
  std::vector<std::vector<std::shared_ptr<Expr>>> to_merge{{}, {}};

  InfoVisitor info_visitor;
  std::visit(info_visitor, *expr);

  const auto expr_type = expr->Type();

  if ((expr_type == ExprKind::kExist || expr_type == ExprKind::kUniversal) &&
      info_visitor.Infos()[0] == src) {
    // If var is bounded by new quantifier, store it to to_merge directly
    // we will then merge it by checking the same condition
    to_merge[1].push_back(std::move(expr));
  }

  // Flatten the AST
  Flatten(flatten, parents, to_merge, src);

  assert(to_merge.size() == flatten.size());

  // Merge back all the changes
  Merge(src, flatten, parents, to_merge, dst);

  return std::move(to_merge[0][0]);
}

[[nodiscard]] auto Expand(std::shared_ptr<Expr> expr, const Token &token)
    -> std::vector<std::vector<std::shared_ptr<Expr>>> {
  ChildrenVisitor children_visitor;
  std::visit(children_visitor, *expr);

  auto &childrens = children_visitor.ViewChildren();

  const auto expr_type = expr->Type();

  if (expr_type == ExprKind::kAnd) { // Alpha expansion
    return {std::move(childrens)};
  }

  if (expr_type == ExprKind::kOr) { // Beta expansion
    return {{std::move(childrens[0])}, {std::move(childrens[1])}};
  }

  if (expr_type == ExprKind::kImpl) { // Beta expansion
    return {{std::make_shared<Expr>(std::in_place_type_t<UnaryExpr>{},
                                    ExprKind::kNeg, std::move(childrens[0]))},
            {std::move(childrens[1])}};
  }

  if (expr_type == ExprKind::kExist || expr_type == ExprKind::kUniversal) {
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
    InfoVisitor info_visitor;
    std::visit(info_visitor, *expr);

    return {{CopyAndReplace(info_visitor.Infos()[0], std::move(childrens[0]),
                            token)}};
  }

  if (expr_type == ExprKind::kNeg) {
    std::shared_ptr<Expr> children = std::move(childrens[0]);

    const auto neg_expr_child_type = children->Type();

    // If we are negating literal, we return Neg+Literal
    // by the definition of literals in Tableau
    if (neg_expr_child_type == ExprKind::kLiteral) {
      return {{std::move(expr)}};
    }

    ChildrenVisitor children_visitor_for_children;
    std::visit(children_visitor_for_children, *children);

    auto &children_of_children = children_visitor_for_children.ViewChildren();

    // If Unary
    // If Neg, we skip the double Negation
    if (neg_expr_child_type == ExprKind::kNeg) {
      return {{std::move(children_of_children[0])}};
    }

    // Otherwise, we negate the Quantified Formula based on their rule
    if (neg_expr_child_type == ExprKind::kExist ||
        neg_expr_child_type == ExprKind::kUniversal) {
      InfoVisitor info_visitor;
      std::visit(info_visitor, *children);

      auto &infos = info_visitor.Infos();
      assert(infos.size() == 1);

      return {{std::make_shared<Expr>(
          std::in_place_type_t<QuantifiedUnaryExpr>{},
          ExprKind::Negate(neg_expr_child_type), std::move(infos[0]),
          std::make_shared<Expr>(std::in_place_type_t<UnaryExpr>{},
                                 ExprKind::kNeg,
                                 std::move(children_of_children[0])))}};
    }

    // If Binary => we negate them based on their rules
    if (ExprKind::IsBinary(neg_expr_child_type)) {
      auto new_children_left =
          neg_expr_child_type == ExprKind::kImpl
              ? std::move(children_of_children[0])
              : std::make_shared<Expr>(std::in_place_type_t<UnaryExpr>{},
                                       ExprKind::kNeg,
                                       std::move(children_of_children[0]));
      auto new_children_right = std::make_shared<Expr>(
          std::in_place_type_t<UnaryExpr>{}, ExprKind::kNeg,
          std::move(children_of_children[1]));
      std::shared_ptr<Expr> node = std::make_shared<Expr>(
          std::in_place_type_t<BinaryExpr>{},
          ExprKind::Negate(neg_expr_child_type), std::move(new_children_left),
          std::move(new_children_right));
      return {{std::move(node)}};
    }
  }

  unreachable();
}
} // namespace

[[nodiscard]] auto TableauFormula::Expand(ConstantManager &manager)
    -> std::vector<std::vector<TableauFormula>> {
  Token token;
  const auto type = Type();

  if (type == ExprKind::kUniversal) {
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
  } else if (type == ExprKind::kExist) {
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
  std::vector expansion = ::Expand(expr_, token);
  std::vector<std::vector<TableauFormula>> ret;
  ret.reserve(expansion.size());

  for (auto &one_expansion : expansion) {
    std::vector<TableauFormula> formulas;
    formulas.reserve(one_expansion.size());
    for (auto &new_formula : one_expansion) {
      formulas.emplace_back(std::move(new_formula));
    }
    ret.push_back(std::move(formulas));
  }

  return ret;
}

auto operator>(const TableauFormula &lhs, const TableauFormula &rhs) -> bool {
  const auto lhs_type = lhs.Type();
  const auto rhs_type = rhs.Type();
  if (lhs_type == rhs_type && lhs_type == ExprKind::kUniversal) {
    return lhs.const_num_ > rhs.const_num_;
  }
  return lhs_type > rhs_type;
}

// An encapsulation of Append
// Help us to filter out literal and Assign formula to their appropriate
// structure
auto Theory::Append(const TableauFormula &formula) -> void {
  const auto formula_type = formula.Type();

  if (ExprKind::IsLiteral(
          formula_type)) { // if tableau literal => literal or neg_literal
    // use description to deal with prop literal and pred literal
    auto literal = formula.Description();
    if (neg_literals_.find(Token{literal}) != neg_literals_.end()) {
      close_ = true;
    } else {
      literals_.emplace(std::move(literal));
    }
    return;
  }

  if (formula_type == ExprKind::kNeg &&
      ExprKind::IsLiteral(formula.ViewChildren()[0].Type())) {
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
  if (formulas_.empty()) {
    return {};
  }

  auto formula = formulas_.top();
  formulas_.pop();

  // Try expanding the formula, if we cannot expand
  //  - reach constant limits
  //  - no more available const for universal formula
  auto expansions = formula.Expand(manager_);

  const auto formula_type = formula.Type();

  /*
    After trying to expand there are only three possibilities:
      - we could expand
      - we could not expand because reaching constant limit (formula.Type() ==
    kExists)
      - we could not expand because universal formula uses all the available
    consts
        - no way for other possible A to generate more consts because the A
          is pop-ed based on their const_num

    In the second case, we need to mark our theory as undecidable
  */
  if (expansions.empty()) {
    if (formula_type == ExprKind::kExist) {
      undecidable_ = true;
    }
    return {};
  }

  std::vector<Theory> new_theories;
  new_theories.reserve(expansions.size());

  for (auto &expansion : expansions) {
    Theory new_theory = *this;
    for (const auto &new_formula : expansion) {
      new_theory.Append(new_formula);
    }

    // Gamma formula needs to be added back to the Theory
    if (formula_type == ExprKind::kUniversal) {
      new_theory.Append(formula);
    }

    new_theories.push_back(std::move(new_theory));
  }

  return new_theories;
}

auto Tableau::Solve(const Parser::ParserOutput &parser_out) -> TableauResult {
  std::vector<Theory> queue;
  queue.emplace_back(TableauFormula(parser_out.GetFormula()));

  bool undecidable{
      false}; // to mark whether we have encountered undecidable formula

  for (decltype(queue)::size_type i = 0; i < queue.size(); ++i) {
    Theory &theory = queue[i];

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
        queue.push_back(std::move(new_theory));
      }
    }
  }

  return undecidable ? TableauResult::kUndecidable
                     : TableauResult::kUnsatisfiable;
}
