#pragma once

#include "exprs/exprs.hh"

class UnaryExpr : public Expr {
public:
  explicit UnaryExpr(enum Type type) : Expr(type) {
    if (!IsUnary(type)) {
      SetError();
      return;
    }
  }
  explicit UnaryExpr(enum Type type, std::shared_ptr<Expr> expr)
      : Expr(type), expr_(std::move(expr)) {}

  auto Append(std::shared_ptr<Expr> expr) -> void final {
    if (expr_ || type_ == Type::kNull) {
      SetError();
      return;
    }

    expr_ = std::move(expr);
  }

  auto Append(enum Type type) -> void final {
    (void)type;
    SetError();
  }

  [[nodiscard]] auto Complete() const -> bool final {
    return type_ != Type::kNull && expr_;
  }

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> final {
    return {expr_};
  }

  [[nodiscard]] auto Infos() const -> std::vector<Token> override { return {}; }

protected:
  std::shared_ptr<Expr> expr_{};
};

// A Special UnaryExpr Expx where E is a quantifier
// Need to handle this case (by checking an additional Token)
class QuantifiedUnaryExpr : public UnaryExpr {
public:
  explicit QuantifiedUnaryExpr(enum Type type, Token var)
      : UnaryExpr(type), var_(std::move(var)) {}
  explicit QuantifiedUnaryExpr(enum Type type, Token var,
                               std::shared_ptr<Expr> expr)
      : UnaryExpr(type, std::move(expr)), var_(std::move(var)) {}

  [[nodiscard]] auto Infos() const -> std::vector<Token> final {
    return {var_};
  }

private:
  Token var_;
};
