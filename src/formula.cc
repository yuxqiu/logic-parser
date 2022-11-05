#include <cstddef>
#include <deque>
#include <utility>

#include "expr.hh"
#include "formula.hh"

Formula::Formula(std::shared_ptr<Expr> expr) : expr_{std::move(expr)} {}

auto Formula::Type() const -> enum Expr::Type { return expr_->Type(); }

void Formula::ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                         std::string &out, Expr *expr) {
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

auto Formula::Description() const -> std::string {
  // Expr*, uint64_t pair => uint64_t stores number of children visited
  std::stack<std::pair<Expr *, uint64_t>> stack;
  std::string out;

  ExpandLeft(stack, out, expr_.get());
  while (!stack.empty()) {
    auto &[expr, num] = stack.top();

    if (num >= expr->ChildrenSize()) {
      Expr::Description(expr, out, num);
      stack.pop();
      continue;
    }

    Expr::Description(expr, out, num);
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
    ret.emplace_back(expr);
  }
  return ret;
}

auto Formula::Expand() const -> std::vector<std::vector<Formula>> {
  std::vector expansion = Expr::Expand(expr_);
  std::vector<std::vector<Formula>> ret;
  ret.reserve(expansion.size());

  for (auto &one_expansion : expansion) {
    std::vector<Formula> formulas;
    formulas.reserve(one_expansion.size());
    for (auto &new_formula : one_expansion) {
      formulas.emplace_back(std::move(new_formula));
    }
    ret.emplace_back(std::move(formulas));
  }

  return ret;
}

void Formula::ReleaseResources() {
  std::deque<std::shared_ptr<Expr>> destruct_queue;
  destruct_queue.emplace_back(std::move(expr_));
  while (!destruct_queue.empty()) {
    auto front = std::move(destruct_queue.front());
    destruct_queue.pop_front();
    if (front.use_count() == 1) {
      auto children = front->ViewChildren();
      for (std::shared_ptr<Expr> &expr : children) {
        destruct_queue.emplace_back(std::move(expr));
      }
    }
  }
}

Formula::~Formula() { ReleaseResources(); }

auto operator<(const Formula &lhs, const Formula &rhs) -> bool {
  return lhs.Type() < rhs.Type();
}