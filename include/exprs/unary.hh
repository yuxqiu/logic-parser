#pragma once

#include <memory>

#include "exprs/kind.hh"
#include "tokenizer.hh"

class Expr;

struct UnaryExpr {
public:
  explicit UnaryExpr(ExprKind type) : type_{type} {}
  explicit UnaryExpr(ExprKind type, std::shared_ptr<Expr> expr)
      : expr_(std::move(expr)), type_{type} {}

  [[nodiscard]] auto Type() const -> ExprKind { return type_; }

  auto Append(std::shared_ptr<Expr> expr) -> void {
    if (expr_) {
      error_ = true;
      return;
    }

    expr_ = std::move(expr);
  }

  auto Append(ExprKind type) -> void {
    (void)type;
    error_ = true;
  }

  [[nodiscard]] auto Complete() const -> bool {
    return type_ != ExprKind::kNull && expr_;
  }

  std::shared_ptr<Expr> expr_{};
  ExprKind type_;
  bool error_ = false;
};

// A Special UnaryExpr Expr where E is a quantifier
// Need to handle this case (by checking an additional Token)
struct QuantifiedUnaryExpr final : public UnaryExpr {
public:
  explicit QuantifiedUnaryExpr(ExprKind type, Token var)
      : UnaryExpr(type), var_(std::move(var)) {}
  explicit QuantifiedUnaryExpr(ExprKind type, Token var,
                               std::shared_ptr<Expr> expr)
      : UnaryExpr(type, std::move(expr)), var_(std::move(var)) {}

  Token var_;
};
