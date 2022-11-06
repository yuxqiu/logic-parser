#pragma once

#include "exprs/exprs.hh"

class UnaryExpr : public Expr {
public:
  explicit UnaryExpr(enum Type type);
  explicit UnaryExpr(enum Type type, std::shared_ptr<Expr> expr);
  ~UnaryExpr() override = default;

  auto Append(std::shared_ptr<Expr> expr) -> void override;
  auto Append(enum Type type) -> void override;

  [[nodiscard]] auto Complete() const -> bool override;

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> override;

  [[nodiscard]] auto Infos() const -> std::vector<Token> override;

protected:
  std::shared_ptr<Expr> expr_{};
};

// A Special UnaryExpr Expx where E is a quantifier
// Need to handle this case (by checking an additional Token)
class QuantifiedUnaryExpr : public UnaryExpr {
public:
  explicit QuantifiedUnaryExpr(enum Type type, Token var);
  explicit QuantifiedUnaryExpr(enum Type type, Token var,
                               std::shared_ptr<Expr> expr);
  ~QuantifiedUnaryExpr() override = default;

  [[nodiscard]] auto Infos() const -> std::vector<Token> override;

private:
  Token var_;
};
