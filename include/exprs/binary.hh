#pragma once

#include "exprs/exprs.hh"

class BinaryExpr : public Expr {
public:
  BinaryExpr() = default;
  explicit BinaryExpr(enum Type type, std::shared_ptr<Expr> expr_lhs,
                      std::shared_ptr<Expr> expr_rhs)
      : Expr(type), expr_lhs_(std::move(expr_lhs)),
        expr_rhs_(std::move(expr_rhs)) {}

  auto Append(std::shared_ptr<Expr> expr) -> void final;
  auto Append(enum Type type) -> void final;

  [[nodiscard]] auto Complete() const -> bool final {
    return type_ != Type::kNull && expr_lhs_ && expr_rhs_;
  }

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> final {
    return {expr_lhs_, expr_rhs_};
  }

  [[nodiscard]] auto Infos() const -> std::vector<Token> final { return {}; }

private:
  std::shared_ptr<Expr> expr_lhs_{}, expr_rhs_{};
};
