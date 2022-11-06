#pragma once

#include <memory>
#include <stack>

#include "formula.hh"
#include "tokenizer.hh"

class Parser {
public:
  enum class ParseResult { kNotAFormula, kProposition, kPredicate };

  class ParserOutput {
  public:
    explicit ParserOutput(Formula &&owner, std::string &&raw_formula,
                          ParseResult result);

    [[nodiscard]] auto Formula() -> Formula &;
    [[nodiscard]] auto Formula() const -> const class Formula &;
    [[nodiscard]] auto RawFormula() const -> const std::string &;
    [[nodiscard]] auto Result() const -> ParseResult;

  private:
    class Formula formula_;
    std::string raw_formula_;
    enum ParseResult result_;
  };

  [[nodiscard]] static auto Parse(std::string line) -> ParserOutput;

private:
  class ExprStack : public std::stack<std::shared_ptr<Expr>> {
  public:
    ~ExprStack();

    [[nodiscard]] auto Error() const -> bool;
    auto SetError() -> void;

  private:
    bool error_{false};
  };

  static auto CleanupStack(ExprStack &stack) -> void;
  static auto MergeStack(ExprStack &stack) -> void;

  static auto ProcessLeftParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token) -> void;
  static auto ProcessRightParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                      Token &token) -> void;
  static auto ProcessBinaryConnective(ExprStack &stack, Tokenizer &tokenizer,
                                      Token &token) -> void;
  static auto ProcessUnaryProp(ExprStack &stack, Tokenizer &tokenizer,
                               Token &token) -> void;
  static auto ProcessLiteralProp(ExprStack &stack, Tokenizer &tokenizer,
                                 Token &token) -> void;
  static auto ProcessUnaryPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                    Token &token) -> void;
  static auto ProcessLiteralPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                      Token &token) -> void;
};