#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "expr.hh"
#include "formula.hh"
#include "parser.hh"
#include "tokenizer.hh"

// Basic Infos
const std::vector<Token> kLeftParenthesisAll = {Token{"("}};
const std::vector<Token> kRightParenthesisAll = {Token{")"}};

const std::vector<Token> kBinaryAll = {Token{"^"}, Token{"v"}, Token{">"}};
const std::map<Token, enum Expr::Type> kBinaryAllToType = {
    {Token{"^"}, Expr::Type::kAnd},
    {Token{"v"}, Expr::Type::kOr},
    {Token{">"}, Expr::Type::kImpl}};

// Prop Starts
const std::vector<Token> kUnaryProp = {Token{"-"}};
const std::map<Token, enum Expr::Type> kUnaryPropToType = {
    {Token{"-"}, Expr::Type::kNeg}};

const std::vector<Token> kLiteralProp = {Token{"p"}, Token{"q"}, Token{"r"},
                                         Token{"s"}};
// Prop Ends

// Predicate Starts
const std::vector<Token> kUnaryPredicate = {Token{"E"}, Token{"A"}};
const std::map<Token, enum Expr::Type> kUnaryPredicateToType = {
    {Token{"E"}, Expr::Type::kExist}, {Token{"A"}, Expr::Type::kUniversal}};

const std::vector<Token> kLiteralPredicate = {Token{"P"}, Token{"Q"},
                                              Token{"R"}, Token{"S"}};
const std::vector<Token> kVarPredicate = {Token{"x"}, Token{"y"}, Token{"z"},
                                          Token{"w"}};
// Predicate Ends

auto Parser::ParserOutput::Formula() -> class Formula & { return formula_; }

auto Parser::ParserOutput::Formula() const -> const class Formula & {
  return formula_;
}

auto Parser::ParserOutput::RawFormula() const -> const std::string & {
  return raw_formula_;
}

auto Parser::ParserOutput::Result() const -> ParseResult { return result_; }

Parser::ParserOutput::ParserOutput(class Formula &&owner,
                                   std::string &&raw_formula,
                                   ParseResult result)
    : formula_(owner), raw_formula_(raw_formula), result_(result) {}

auto Parser::ExprStack::Error() const -> bool { return error_; }

auto Parser::ExprStack::SetError() -> void { error_ = true; }

Parser::ExprStack::~ExprStack() {
  while (!empty()) {
    const Formula destructed_formula{std::move(top())};
    pop();
  }
}

/**
 * On success, return true; if encounter any error during merging, return false
 */
auto Parser::MergeStack(ExprStack &stack) -> void {
  // First merge always come from 1) a ')' or 2) a literal
  // This can always be merged
  if (stack.size() > 1 && stack.top()->Complete()) {
    std::shared_ptr<Expr> expr = std::move(stack.top());
    stack.pop();
    stack.top()->Append(expr);

    // If Merge Error, place expr back and mark error
    if (stack.top()->Error()) {
      stack.emplace(std::move(expr));
      stack.SetError();
    }
  }

  // If we want to continue to merge,
  // we need to ensure we do not eliminate other BinaryConnective
  while (stack.size() > 1 && stack.top()->Complete() &&
         !Expr::IsBinary(stack.top()->Type())) {
    std::shared_ptr<Expr> expr = std::move(stack.top());
    stack.pop();
    stack.top()->Append(expr);

    // If Merge Error, place expr back and mark error
    if (stack.top()->Error()) {
      stack.emplace(std::move(expr));
      stack.SetError();
      break;
    }
  }
}

auto Parser::ProcessLeftParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                    Token &token) -> void {
  (void)tokenizer;
  (void)token;
  // If ( => a new BinaryExpr
  stack.emplace(std::make_shared<BinaryExpr>());
}

auto Parser::ProcessRightParenthesis(ExprStack &stack, Tokenizer &tokenizer,
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

auto Parser::ProcessBinaryConnective(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token) -> void {
  (void)tokenizer;

  // if these, we set an error
  //  - stack is empty (Binary Connective appears before any Formula)
  if (stack.empty()) {
    stack.SetError();
    return;
  }

  const enum Expr::Type type = kBinaryAllToType.at(token);

  // If the BinaryExpr cannot be added to the top formula
  // e.g. top is a Literal/Unary
  // the top formula will be set an error state by its Append
  stack.top()->Append(type);
}

auto Parser::ProcessUnaryProp(ExprStack &stack, Tokenizer &tokenizer,
                              Token &token) -> void {
  (void)tokenizer;
  // If UnaryProp (-) => create a new Unary Expr
  enum Expr::Type type = kUnaryPropToType.at(token);
  stack.emplace(std::make_shared<UnaryExpr>(type));
}

auto Parser::ProcessLiteralProp(ExprStack &stack, Tokenizer &tokenizer,
                                Token &token) -> void {
  (void)tokenizer;
  // If Literal => create a new literal
  stack.emplace(std::make_shared<Literal>(std::move(token)));
  MergeStack(stack);
}

auto Parser::ProcessUnaryPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                   Token &token) -> void {
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
      kUnaryPredicateToType.at(token), std::move(next_token)));
}

// Process formulas like P(x,y)
auto Parser::ProcessLiteralPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token) -> void {
  (void)tokenizer;
  std::vector<Token> token_holder;
  token_holder.reserve(5);

  // Hard coded way to process Predicate Formulas
  for (int i = 0; i < 5; ++i) {
    if (tokenizer.Empty()) {
      stack.SetError();
      return;
    }

    token_holder.emplace_back(tokenizer.PeekToken());
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
  if (stack.Error() || stack.size() != 1 || (proposition == predicate) ||
      !stack.top()->Complete() || stack.top()->Error()) {
    return ParserOutput{Formula{}, std::move(line), ParseResult::kNotAFormula};
  }

  return ParserOutput{Formula{std::move(stack.top())}, std::move(line),
                      proposition ? ParseResult::kProposition
                                  : ParseResult::kPredicate};
}