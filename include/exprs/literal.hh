#pragma once

#include "exprs/exprs.hh"

// A Special Literal -p where p is literal
// Need to handle this case - Use decorator later!
class Literal : public Expr {
public:
  explicit Literal(Token val);
  ~Literal() override = default;
  Literal(Literal &&) = default;

  auto Append(std::shared_ptr<Expr> expr) -> void override;
  auto Append(enum Type type) -> void override;

  [[nodiscard]] auto Complete() const -> bool override;

  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> override;

  [[nodiscard]] auto Infos() const -> std::vector<Token> override;

protected:
  Token val_;
};

class PredicateLiteral : public Literal {
public:
  explicit PredicateLiteral(Token val, Token left_var_, Token right_var_);
  ~PredicateLiteral() override = default;
  PredicateLiteral(PredicateLiteral &&) = default;

  [[nodiscard]] auto Infos() const -> std::vector<Token> override;

  Token left_var_, right_var_;
};