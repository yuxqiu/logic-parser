#include <memory>
#include <queue>
#include <vector>

#include "expr.hh"
#include "formula.hh"

FormulaViewer::FormulaViewer(Expr *expr) : expr_{expr} {}

auto FormulaViewer::Type() const -> enum Expr::Type { return expr_->Type(); }

auto FormulaViewer::Description() const -> std::string {
  return expr_->Description();
}

auto FormulaViewer::ViewChildren() const -> std::vector<FormulaViewer> {
  std::vector children = expr_->ViewChildren();
  std::vector<FormulaViewer> ret;
  ret.reserve(children.size());
  for (auto &expr : children) {
    ret.emplace_back(expr.get());
  }
  return ret;
}

FormulaOwner::FormulaOwner(std::shared_ptr<Expr> &&expr)
    : expr_(std::move(expr)) {}

void FormulaOwner::ReleaseResources() {
  std::queue<std::shared_ptr<Expr>> destruct_queue;
  destruct_queue.emplace(std::move(expr_));
  while (!destruct_queue.empty()) {
    auto front = std::move(destruct_queue.front());
    destruct_queue.pop();
    if (front) {
      auto children = front->TakeChildren();
      for (std::shared_ptr<Expr> &expr : children) {
        destruct_queue.emplace(std::move(expr));
      }
    }
  }
}

FormulaOwner::~FormulaOwner() { ReleaseResources(); }

auto FormulaOwner::operator=(FormulaOwner &&owner) noexcept -> FormulaOwner & {
  if (this != &owner) {
    ReleaseResources();
    expr_ = std::move(owner.expr_);
  }
  return *this;
}

FormulaOwner::operator FormulaViewer() const {
  return FormulaViewer{expr_.get()};
}