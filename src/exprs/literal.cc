#include "exprs/literal.hh"

Literal::Literal(Token val) : Expr(Type::kLiteral), val_{std::move(val)} {}

auto Literal::Append(std::shared_ptr<Expr> expr) -> void {
  (void)expr;
  SetError();
}

auto Literal::Append(enum Type type) -> void {
  (void)type;
  SetError();
}

[[nodiscard]] auto Literal::ViewChildren() const
    -> std::vector<std::shared_ptr<Expr>> {
  return {};
}

[[nodiscard]] auto Literal::Infos() const -> std::vector<Token> {
  return {val_};
}

[[nodiscard]] auto Literal::Complete() const -> bool { return true; }

PredicateLiteral::PredicateLiteral(Token val, Token left, Token right)
    : Literal(std::move(val)), left_var_(std::move(left)),
      right_var_(std::move(right)) {}

[[nodiscard]] auto PredicateLiteral::Infos() const -> std::vector<Token> {
  return {val_, left_var_, right_var_};
}