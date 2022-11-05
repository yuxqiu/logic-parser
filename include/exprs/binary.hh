#pragma once

#include "exprs/exprs.hh"

class BinaryExpr : public Expr {
public:
  BinaryExpr() = default;
  explicit BinaryExpr(enum Type type, std::shared_ptr<Expr> expr_lhs,
                      std::shared_ptr<Expr> expr_rhs);
  ~BinaryExpr() override = default;

  void Append(std::shared_ptr<Expr> expr) override;
  void Append(enum Type type) override;

  [[nodiscard]] auto Complete() const -> bool override;

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> override;

  [[nodiscard]] auto Infos() const -> std::vector<Token> override;

private:
  std::shared_ptr<Expr> expr_lhs_{}, expr_rhs_{};
};
