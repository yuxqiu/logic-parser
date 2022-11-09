#include <cstddef>
#include <deque>
#include <utility>

#include "expr.hh"
#include "formula.hh"

Formula::Formula(std::shared_ptr<Expr> expr) : expr_{std::move(expr)} {}

auto Formula::Type() const -> enum Expr::Type { return expr_->Type(); }

// Add all the left child into the stack
//
// Left child is defined as
//  - no left child for literal
//  - the only child of the UnaryExpr
//  - the left child of the BinaryExpr
auto Formula::ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                         std::string &out, Expr *expr) -> void {
  while (expr != nullptr) {
    Expr::Description(expr, out, 0);
    stack.emplace(expr, 1);
    switch (expr->ChildrenSize()) {
    case 0:
      expr = nullptr;
      break;
    case 1:
      [[fallthrough]];
    case 2:
      expr = expr->ViewChildren()[0].get();
    }
  }
}

/*
  To "reassemble" the formula from the tree iteratively, we need to
  ask different exprs to provide information at three stages
    - when no child is visited: '(', '-', 'E', 'A'
    - when one child is visited: '^', 'v', '>'
    - when two children are visited: ")"

  So, we design a modified iterative in-order traversal where
  each expr has at most three stages (0, 1, 2 children are visited).
  After all children are processed, it will be pop out of the stack
*/
auto Formula::Description() const -> std::string {
  // Expr*, uint64_t pair => uint64_t stores number of children visited
  std::stack<std::pair<Expr *, uint64_t>> stack;
  std::string out;

  ExpandLeft(stack, out, expr_.get());
  while (!stack.empty()) {
    auto &[expr, num] = stack.top();

    Expr::Description(expr, out, num);
    if (num >= expr->ChildrenSize()) {
      stack.pop();
      continue;
    }

    // Only BinaryExpr falls into this case
    // because it has ChildrenSize == 2
    ExpandLeft(stack, out, expr->ViewChildren()[num].get());
    ++num;
  }

  return out;
}

auto Formula::Infos() const -> std::vector<Token> { return expr_->Infos(); }

auto Formula::ViewChildren() const -> std::vector<Formula> {
  std::vector children = expr_->ViewChildren();
  std::vector<Formula> ret;
  ret.reserve(children.size());
  for (auto &expr : children) {
    ret.emplace_back(std::move(expr));
  }
  return ret;
}

/*
  Relying on the destructor of Expr is dangerous, as it may
    - cause stack overflow if the Expr is long enough

  ReleaseResources propose a iterative way to destruct all the formula
  by using a deque

  It will only destruct the formula only if use_count of current formula is == 1
    - if use_count == 0, it's already destroyed
    - if use_count > 1, some other Formulas are also managing this expr,
      so we don't need to destruct it
*/
auto Formula::ReleaseResources() -> void {
  std::deque<std::shared_ptr<Expr>> destruct_queue;
  destruct_queue.emplace_back(std::move(expr_));
  while (!destruct_queue.empty()) {
    const auto front = std::move(destruct_queue.front());
    destruct_queue.pop_front();
    if (front.use_count() == 1) {
      // increment ref_count, so it will not be destroyed after front goes out
      // of scope
      auto children = front->ViewChildren();
      for (std::shared_ptr<Expr> &expr : children) {
        destruct_queue.emplace_back(std::move(expr));
      }
    }
  }
}

Formula::~Formula() { ReleaseResources(); }