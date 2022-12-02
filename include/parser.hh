#pragma once

#include <memory>
#include <stack>

#include "exprs/exprs.hh"
#include "formula.hh"
#include "tokenizer.hh"

class Parser {
public:
  enum class ParseResult { kNotAFormula, kProposition, kPredicate };

  class ParserOutput {
  public:
    explicit ParserOutput(Formula owner, std::string raw_formula,
                          ParseResult result)
        : formula_(std::move(owner)), raw_formula_(std::move(raw_formula)),
          result_(result) {}

    [[nodiscard]] auto Formula() -> Formula & { return formula_; }
    [[nodiscard]] auto Formula() const -> const class Formula & {
      return formula_;
    }
    [[nodiscard]] auto RawFormula() const -> const std::string & {
      return raw_formula_;
    }
    [[nodiscard]] auto Result() const -> ParseResult { return result_; }

  private:
    class Formula formula_;
    std::string raw_formula_;
    enum ParseResult result_;
  };

  [[nodiscard]] static auto Parse(std::string line) -> ParserOutput;

private:
  class ExprStack : std::stack<std::shared_ptr<Expr>> {
  public:
    using std::stack<std::shared_ptr<Expr>>::empty;
    using std::stack<std::shared_ptr<Expr>>::top;
    using std::stack<std::shared_ptr<Expr>>::pop;
    using std::stack<std::shared_ptr<Expr>>::emplace;

    ExprStack() = default;
    ~ExprStack();
    ExprStack(const ExprStack &) = delete;
    ExprStack(ExprStack &&) = delete;
    auto operator=(const ExprStack &) -> ExprStack & = delete;
    auto operator=(ExprStack &&) -> ExprStack & = delete;

    [[nodiscard]] auto Error() const -> bool { return error_; }
    auto SetError() -> void { error_ = true; }

    auto Holder() -> std::shared_ptr<Expr> & { return holder_; }

  private:
    std::shared_ptr<Expr> holder_{};
    bool error_{false};
  };

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

  static auto Merge(ExprStack &stack) -> void;
  static auto MergeStack(ExprStack &stack) -> void;
};