#include "exprs/literal.hh"

Literal::Literal(Token val) : val_{std::move(val)} { type_ = Type::kLiteral; }

void Literal::Append(std::shared_ptr<Expr> expr) {
  (void)expr;
  SetError();
}

void Literal::Append(enum Type type) {
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