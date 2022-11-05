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
    [[nodiscard]] auto Result() const -> ParseResult;
    [[nodiscard]] auto RawFormula() const -> const std::string &;

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
    void SetError();

  private:
    bool error_{false};
  };

  static void CleanupStack(ExprStack &stack);
  static void MergeStack(ExprStack &stack);

  static void ProcessLeftParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token);
  static void ProcessRightParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                      Token &token);
  static void ProcessBinaryConnective(ExprStack &stack, Tokenizer &tokenizer,
                                      Token &token);
  static void ProcessUnaryProp(ExprStack &stack, Tokenizer &tokenizer,
                               Token &token);
  static void ProcessLiteralProp(ExprStack &stack, Tokenizer &tokenizer,
                                 Token &token);
  static void ProcessUnaryPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                    Token &token);
  static void ProcessLiteralPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                      Token &token);
};