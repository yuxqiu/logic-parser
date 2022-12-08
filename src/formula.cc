#include <cassert>
#include <cstddef>
#include <stack>
#include <utility>
#include <vector>

#include "exprs/exprs.hh"
#include "formula.hh"
#include "visitor/children_visitor.hh"
#include "visitor/info_visitor.hh"

namespace {
auto TypeToString(enum Expr::Type type) -> std::string {
  switch (type) {
  case Expr::Type::kAnd:
    return "^";
  case Expr::Type::kOr:
    return "v";
  case Expr::Type::kImpl:
    return ">";
  case Expr::Type::kNeg:
    return "-";
  case Expr::Type::kExist:
    return "E";
  case Expr::Type::kUniversal:
    return "A";
  case Expr::Type::kNull:
  case Expr::Type::kLiteral:
    break;
  }

  assert(false);
  return "Unreachable";
}

auto LiteralDescription(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (num == 0) {
    InfoVisitor visitor;
    expr->Accept(visitor);

    auto &&infos = visitor.Infos();
    out += infos[0].ToString();
    if (infos.size() == 3) {
      out += "(" + infos[1].ToString() + "," + infos[2].ToString() + ")";
    }
  }
}

auto UnaryDescription(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (num == 0) {
    out += TypeToString(expr->Type());
    if (expr->Type() == Expr::Type::kExist ||
        expr->Type() == Expr::Type::kUniversal) {
      InfoVisitor visitor;
      expr->Accept(visitor);
      out += visitor.Infos()[0].ToString();
    }
  }
}

auto BinaryDescription(const Expr *expr, std::string &out, uint64_t num)
    -> void {
  if (num == 0) {
    out += "(";
  } else if (num == 1) {
    out += TypeToString(expr->Type());
  } else if (num == 2) {
    out += ")";
  }
}

auto Description(const Expr *expr, std::string &out, uint64_t num) -> void {
  if (Expr::IsBinary(expr->Type())) {
    BinaryDescription(expr, out, num);
  } else if (Expr::IsUnary(expr->Type())) {
    UnaryDescription(expr, out, num);
  } else if (Expr::IsLiteral(expr->Type())) {
    LiteralDescription(expr, out, num);
  }
}

// Add all the left child into the stack
//
// Left child is defined as
//  - no left child for literal
//  - the only child of the UnaryExpr
//  - the left child of the BinaryExpr
auto ExpandLeft(std::stack<std::pair<Expr *, uint64_t>> &stack,
                std::string &out, Expr *expr) -> void {
  while (true) {
    ::Description(expr, out, 0);
    stack.emplace(expr, 1);

    ChildrenVisitor children_visitor;
    expr->Accept(children_visitor);

    switch (children_visitor.ChildrenSize()) {
    case 0:
      return;
    case 1:
      [[fallthrough]];
    case 2:
      expr = children_visitor.ViewChildren()[0].get();
    }
  }
}
} // namespace

[[nodiscard]] auto Formula::Connective() const -> std::string {
  return TypeToString(expr_->Type());
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

    ::Description(expr, out, num);

    ChildrenVisitor children_visitor;
    expr->Accept(children_visitor);

    if (num >= children_visitor.ChildrenSize()) {
      stack.pop();
      continue;
    }

    // Only BinaryExpr falls into this case
    // because it has ChildrenSize == 2
    //
    // Need to increment num here as the ExpandLeft operation may change the
    // place where num is stored
    ExpandLeft(stack, out, children_visitor.ViewChildren()[num++].get());
  }

  return out;
}

auto Formula::ViewChildren() const -> std::vector<Formula> {
  ChildrenVisitor children_visitor;
  expr_->Accept(children_visitor);
  std::vector<Formula> ret;
  ret.reserve(children_visitor.ChildrenSize());
  for (auto &&expr : children_visitor.ViewChildren()) {
    ret.emplace_back(std::move(expr));
  }
  return ret;
}

/*
  Relying on the destructor of Expr is dangerous, as it may
    - cause stack overflow if the Expr is long enough

  Destructor of Formula proposes a iterative way to destruct all the formula
  by using a deque

  It will only destruct the formula only if use_count of current formula is == 1
    - if use_count == 0, it's already destroyed
    - if use_count > 1, some other Formulas are also managing this expr,
      so we don't need to destruct it
*/
Formula::~Formula() {
  std::vector<std::shared_ptr<Expr>> destruct_queue;
  destruct_queue.emplace_back(std::move(expr_));
  for (decltype(destruct_queue)::size_type i = 0; i < destruct_queue.size();
       ++i) {
    const auto front = std::move(destruct_queue[i]);
    if (front.use_count() == 1) {
      ChildrenVisitor children_visitor;
      front->Accept(children_visitor);
      for (auto &&expr : children_visitor.ViewChildren()) {
        destruct_queue.emplace_back(std::move(expr));
      }
    }
  }
}