#include <algorithm>
#include <array>
#include <cassert>
#include <memory>

#include "exprs/binary.hh"
#include "exprs/exprs.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"
#include "formula.hh"
#include "parser.hh"
#include "tokenizer.hh"

namespace {
// Basic Infos
const std::array kLeftParenthesisAll = {Token{"("}};
const std::array kRightParenthesisAll = {Token{")"}};

const std::array kBinaryAll = {Token{"^"}, Token{"v"}, Token{">"}};
const std::array kBinaryAllToType = {std::pair{Token{"^"}, Expr::Type::kAnd},
                                     std::pair{Token{"v"}, Expr::Type::kOr},
                                     std::pair{Token{">"}, Expr::Type::kImpl}};

// Prop Starts
const std::array kUnaryProp = {Token{"-"}};
const std::array kUnaryPropToType = {std::pair{Token{"-"}, Expr::Type::kNeg}};

const std::array kLiteralProp = {Token{"p"}, Token{"q"}, Token{"r"},
                                 Token{"s"}};
// Prop Ends

// Predicate Starts
const std::array kUnaryPredicate = {Token{"E"}, Token{"A"}};
const std::array kUnaryPredicateToType = {
    std::pair{Token{"E"}, Expr::Type::kExist},
    std::pair{Token{"A"}, Expr::Type::kUniversal}};

const std::array kLiteralPredicate = {Token{"P"}, Token{"Q"}, Token{"R"},
                                      Token{"S"}};
const std::array kVarPredicate = {Token{"x"}, Token{"y"}, Token{"z"},
                                  Token{"w"}};
// Predicate Ends

class ExprStack : std::stack<std::shared_ptr<Expr>> {
public:
  using std::stack<std::shared_ptr<Expr>>::empty;
  using std::stack<std::shared_ptr<Expr>>::top;
  using std::stack<std::shared_ptr<Expr>>::pop;
  using std::stack<std::shared_ptr<Expr>>::emplace;

  ExprStack() = default;
  ~ExprStack() {
    while (!empty()) {
      [[maybe_unused]] Formula expr_destructor{std::move(top())};
      pop();
    }
  }
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

auto Merge(ExprStack &stack) -> void {
  auto expr = std::move(stack.top());
  stack.pop();

  if (stack.empty()) {
    if (stack.Holder()) {
      stack.SetError();
    } else {
      stack.Holder() = std::move(expr);
    }
  } else {
    stack.top()->Append(expr);
    if (stack.top()->Error()) {
      stack.emplace(std::move(expr));
      stack.SetError();
    }
  }
}

auto MergeStack(ExprStack &stack) -> void {
  Merge(stack);
  while (!stack.empty() && !stack.Error() &&
         !Expr::IsBinary(stack.top()->Type()) && stack.top()->Complete()) {
    Merge(stack);
  }
}

auto ProcessLeftParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                            Token &token) -> void {
  (void)tokenizer;
  (void)token;
  // If ( => a new BinaryExpr
  stack.emplace(std::make_shared<BinaryExpr>());
}

auto ProcessRightParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                             Token &token) -> void {
  (void)tokenizer;
  (void)token;
  // if these, we set an error
  //  - stack is empty
  //  - stack.top is not Complete
  //  - stack.top.type is not Binary Expr
  if (stack.empty() || !stack.top()->Complete() ||
      !Expr::IsBinary(stack.top()->Type())) {
    stack.SetError();
    return;
  }

  // we can merge at least one level of the stack
  MergeStack(stack);
}

auto ProcessBinaryConnective(ExprStack &stack, Tokenizer &tokenizer,
                             Token &token) -> void {
  (void)tokenizer;

  // if these, we set an error
  //  - stack is empty (Binary Connective appears before any Formula)
  if (stack.empty()) {
    stack.SetError();
    return;
  }

  const enum Expr::Type type =
      std::find_if(kBinaryAllToType.begin(), kBinaryAllToType.end(),
                   [&token](const std::pair<Token, enum Expr::Type> &lhs) {
                     return lhs.first == token;
                   })
          ->second;

  // If the BinaryExpr cannot be added to the top formula
  // e.g. top is a Literal/Unary
  // the top formula will be set an error state by its Append
  stack.top()->Append(type);
}

auto ProcessUnaryProp(ExprStack &stack, Tokenizer &tokenizer, Token &token)
    -> void {
  (void)tokenizer;
  // If UnaryProp (-) => create a new Unary Expr
  const enum Expr::Type type =
      std::find_if(kUnaryPropToType.begin(), kUnaryPropToType.end(),
                   [&token](const std::pair<Token, enum Expr::Type> &lhs) {
                     return lhs.first == token;
                   })
          ->second;
  stack.emplace(std::make_shared<UnaryExpr>(type));
}

auto ProcessLiteralProp(ExprStack &stack, Tokenizer &tokenizer, Token &token)
    -> void {
  (void)tokenizer;
  // If Literal => create a new literal
  stack.emplace(std::make_shared<Literal>(std::move(token)));
  MergeStack(stack);
}

auto ProcessUnaryPredicate(ExprStack &stack, Tokenizer &tokenizer, Token &token)
    -> void {
  (void)tokenizer;
  Token next_token = tokenizer.PeekToken();
  tokenizer.PopToken();

  // If the bounded var cannot be founded in accepted var => set an error
  if (std::find(kVarPredicate.begin(), kVarPredicate.end(), next_token) ==
      kVarPredicate.end()) {
    stack.SetError();
    return;
  }

  stack.emplace(std::make_shared<QuantifiedUnaryExpr>(
      std::find_if(kUnaryPredicateToType.begin(), kUnaryPredicateToType.end(),
                   [&token](const std::pair<Token, enum Expr::Type> &lhs) {
                     return lhs.first == token;
                   })
          ->second,
      std::move(next_token)));
}

// Process formulas like P(x,y)
auto ProcessLiteralPredicate(ExprStack &stack, Tokenizer &tokenizer,
                             Token &token) -> void {
  (void)tokenizer;
  std::array<Token, 5> token_holder;

  // Hard coded way to process Predicate Formulas
  for (std::array<Token, 5>::size_type i = 0; i < 5; ++i) {
    if (tokenizer.Empty()) {
      stack.SetError();
      return;
    }

    token_holder[i] = tokenizer.PeekToken();
    tokenizer.PopToken();
  }

  if (!(token_holder[0] == Token{"("}) || !(token_holder[2] == Token{","}) ||
      !(token_holder[4] == Token{")"})) {
    stack.SetError();
    return;
  }

  // Check the variables inside the literal are valid
  if (std::find(kVarPredicate.begin(), kVarPredicate.end(), token_holder[1]) ==
          kVarPredicate.end() ||
      std::find(kVarPredicate.begin(), kVarPredicate.end(), token_holder[3]) ==
          kVarPredicate.end()) {
    stack.SetError();
    return;
  }

  stack.emplace(std::make_shared<PredicateLiteral>(std::move(token),
                                                   std::move(token_holder[1]),
                                                   std::move(token_holder[3])));
  MergeStack(stack);
}
} // namespace

auto Parser::Parse(std::string line) -> ParserOutput {
  bool proposition{false};
  bool predicate{false};

  Tokenizer tokenizer{line};
  ExprStack stack;

  /**
   * Extract token until 1) no more token or 2) we get a formula that is a prop
     and also pred
   *
   * Dispatch each token to its corresponding event
   */
  while (!tokenizer.Empty() && (!proposition || !predicate)) {
    if (stack.Error() || (!stack.empty() && stack.top()->Error())) {
      break;
    }

    // Get new Token
    Token token = tokenizer.PeekToken();
    tokenizer.PopToken();

    // Process new Token
    if (std::find(kLeftParenthesisAll.begin(), kLeftParenthesisAll.end(),
                  token) != kLeftParenthesisAll.end()) {
      ProcessLeftParenthesis(stack, tokenizer, token);
      continue;
    }

    if (std::find(kRightParenthesisAll.begin(), kRightParenthesisAll.end(),
                  token) != kRightParenthesisAll.end()) {
      ProcessRightParenthesis(stack, tokenizer, token);
      continue;
    }

    if (std::find(kBinaryAll.begin(), kBinaryAll.end(), token) !=
        kBinaryAll.end()) {
      ProcessBinaryConnective(stack, tokenizer, token);
      continue;
    }

    if (std::find(kUnaryProp.begin(), kUnaryProp.end(), token) !=
        kUnaryProp.end()) {
      ProcessUnaryProp(stack, tokenizer, token);
      continue;
    }

    if (std::find(kLiteralProp.begin(), kLiteralProp.end(), token) !=
        kLiteralProp.end()) {
      ProcessLiteralProp(stack, tokenizer, token);
      proposition = true;
      continue;
    }

    /* quantifier here */
    if (std::find(kUnaryPredicate.begin(), kUnaryPredicate.end(), token) !=
        kUnaryPredicate.end()) {
      ProcessUnaryPredicate(stack, tokenizer, token);
      predicate = true;
      continue;
    }

    // literal in predicate logic
    if (std::find(kLiteralPredicate.begin(), kLiteralPredicate.end(), token) !=
        kLiteralPredicate.end()) {
      ProcessLiteralPredicate(stack, tokenizer, token);
      predicate = true;
      continue;
    }

    // no match
    stack.SetError();
  }

  /*
    if
      - stack is on error (the last action triggers error)
      - stack has more than one formula (unbalanced bracket)
      - both prop and pred (has symbol in both prop and pred - incorrect syntax)
      - top formula is not complete (missing symbol - incorrect syntax)
      - top formula is in error (unmatched symbol - incorrect syntax)
  */
  auto expr = stack.Holder();
  if (stack.Error() || !stack.empty() || (proposition == predicate) || !expr) {
    return ParserOutput{Formula{}, std::move(line), ParseResult::kNotAFormula};
  }

  return ParserOutput{Formula{std::move(expr)}, std::move(line),
                      proposition ? ParseResult::kProposition
                                  : ParseResult::kPredicate};
}