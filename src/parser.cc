#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <stack>
#include <string_view>
#include <utility>

#include "exprs/binary.hh"
#include "exprs/expr.hh"
#include "exprs/literal.hh"
#include "exprs/unary.hh"
#include "formula.hh"
#include "parser.hh"
#include "tokenizer.hh"

namespace {
using namespace std::literals;
// Basic Infos
constexpr std::array kLeftParenthesisAll = {"("sv};
constexpr std::array kRightParenthesisAll = {")"sv};

constexpr std::array kBinaryAll = {"^"sv, "v"sv, ">"sv};
constexpr std::array kBinaryAllToType = {std::pair{"^"sv, ExprKind::kAnd},
                                         std::pair{"v"sv, ExprKind::kOr},
                                         std::pair{">"sv, ExprKind::kImpl}};

// Prop Starts
constexpr std::array kUnaryProp = {"-"sv};
constexpr std::array kUnaryPropToType = {std::pair{"-"sv, ExprKind::kNeg}};

constexpr std::array kLiteralProp = {"p"sv, "q"sv, "r"sv, "s"sv};
// Prop Ends

// Predicate Starts
constexpr std::array kUnaryPredicate = {"E"sv, "A"sv};
constexpr std::array kUnaryPredicateToType = {
    std::pair{"E"sv, ExprKind::kExist}, std::pair{"A"sv, ExprKind::kUniversal}};

constexpr std::array kLiteralPredicate = {"P"sv, "Q"sv, "R"sv, "S"sv};
constexpr std::array kVarPredicate = {"x"sv, "y"sv, "z"sv, "w"sv};
// Predicate Ends

class ExprStack : std::stack<std::shared_ptr<Expr>> {
public:
  using std::stack<std::shared_ptr<Expr>>::empty;
  using std::stack<std::shared_ptr<Expr>>::top;
  using std::stack<std::shared_ptr<Expr>>::pop;
  using std::stack<std::shared_ptr<Expr>>::push;

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
      stack.push(std::move(expr));
      stack.SetError();
    }
  }
}

auto MergeStack(ExprStack &stack) -> void {
  Merge(stack);
  while (!stack.empty() && !stack.Error() &&
         !ExprKind::IsBinary(stack.top()->Type()) && stack.top()->Complete()) {
    Merge(stack);
  }
}

auto ProcessLeftParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                            Token &token) -> void {
  (void)tokenizer;
  (void)token;
  // If ( => a new BinaryExpr
  stack.push(std::make_shared<Expr>(std::in_place_type_t<BinaryExpr>{}));
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
      !ExprKind::IsBinary(stack.top()->Type())) {
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

  const ExprKind type =
      std::find_if(kBinaryAllToType.begin(), kBinaryAllToType.end(),
                   [&token](const std::pair<std::string_view, ExprKind> &lhs) {
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
  const ExprKind type =
      std::find_if(kUnaryPropToType.begin(), kUnaryPropToType.end(),
                   [&token](const std::pair<std::string_view, ExprKind> &lhs) {
                     return lhs.first == token;
                   })
          ->second;
  stack.push(std::make_shared<Expr>(std::in_place_type_t<UnaryExpr>{}, type));
}

auto ProcessLiteralProp(ExprStack &stack, Tokenizer &tokenizer, Token &token)
    -> void {
  (void)tokenizer;
  // If Literal => create a new literal
  stack.push(std::make_shared<Expr>(std::in_place_type_t<Literal>{},
                                    std::move(token)));
  MergeStack(stack);
}

auto ProcessUnaryPredicate(ExprStack &stack, Tokenizer &tokenizer, Token &token)
    -> void {
  if (tokenizer.Empty()) {
    stack.SetError();
    return;
  }

  Token next_token = tokenizer.PeekToken();
  tokenizer.PopToken();

  // If the bounded var cannot be founded in accepted var => set an error
  if (std::find(kVarPredicate.begin(), kVarPredicate.end(), next_token) ==
      kVarPredicate.end()) {
    stack.SetError();
    return;
  }

  stack.push(std::make_shared<Expr>(
      std::in_place_type_t<QuantifiedUnaryExpr>{},
      std::find_if(kUnaryPredicateToType.begin(), kUnaryPredicateToType.end(),
                   [&token](const std::pair<std::string_view, ExprKind> &lhs) {
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

  if (!(token_holder[0] == "("sv) || !(token_holder[2] == ","sv) ||
      !(token_holder[4] == ")"sv)) {
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

  stack.push(std::make_shared<Expr>(
      std::in_place_type_t<PredicateLiteral>{}, std::move(token),
      std::move(token_holder[1]), std::move(token_holder[3])));
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
  auto &expr = stack.Holder();
  if (stack.Error() || !stack.empty() || (proposition == predicate) || !expr) {
    return ParserOutput{{}, std::move(line), ParseResult::kNotAFormula};
  }

  return ParserOutput{Formula{std::move(expr)}, std::move(line),
                      proposition ? ParseResult::kProposition
                                  : ParseResult::kPredicate};
}
