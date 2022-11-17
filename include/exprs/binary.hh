#pragma once

#include "exprs/exprs.hh"

class BinaryExpr : public Expr {
public:
  BinaryExpr() = default;
  explicit BinaryExpr(enum Type type, std::shared_ptr<Expr> expr_lhs,
                      std::shared_ptr<Expr> expr_rhs);

  auto Append(std::shared_ptr<Expr> expr) -> void final;
  auto Append(enum Type type) -> void final;

  [[nodiscard]] auto Complete() const -> bool final;

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> final;

  [[nodiscard]] auto Infos() const -> std::vector<Token> final;

private:
  std::shared_ptr<Expr> expr_lhs_{}, expr_rhs_{};
};
