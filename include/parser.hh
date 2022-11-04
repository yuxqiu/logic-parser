#pragma once

#include <iostream>
#include <memory>
#include <stack>

#include "formula.hh"
#include "tokenizer.hh"

class Parser {
public:
  enum class ParseResult { kNotAFormula, kProposition, kPredicate };

  class ParserOutput {
  public:
    explicit ParserOutput(FormulaOwner &&owner, std::string &&raw_formula,
                          ParseResult result);

    [[nodiscard]] auto Formula() -> FormulaOwner &;
    [[nodiscard]] auto Formula() const -> const FormulaOwner &;
    [[nodiscard]] auto Result() const -> ParseResult;
    [[nodiscard]] auto RawFormula() const -> const std::string &;

  private:
    FormulaOwner formula_;
    std::string raw_formula_;
    enum ParseResult result_;
  };

  [[nodiscard]] static auto Parse(std::string line) -> ParserOutput;

  static void PrintInformation(std::ostream &out,
                               const ParserOutput &parser_out);

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