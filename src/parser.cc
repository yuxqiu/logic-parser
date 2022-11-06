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

auto Parser::ParserOutput::Result() const -> ParseResult { return result_; }

Parser::ParserOutput::ParserOutput(class Formula &&owner, ParseResult result)
    : formula_(owner), result_(result) {}

auto Parser::ExprStack::Error() const -> bool { return error_; }

void Parser::ExprStack::SetError() { error_ = true; }

Parser::ExprStack::~ExprStack() {
  while (!empty()) {
    Formula destructed_formula{std::move(top())};
    pop();
  }
}

/**
 * On success, return true; if encounter any error during merging, return false
 */
void Parser::MergeStack(ExprStack &stack) {
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

void Parser::ProcessLeftParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                    Token &token) {
  (void)tokenizer;
  (void)token;
  stack.emplace(std::make_shared<BinaryExpr>());
}

void Parser::ProcessRightParenthesis(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token) {
  (void)tokenizer;
  (void)token;
  if (stack.empty() || !stack.top()->Complete() ||
      !Expr::IsBinary(stack.top()->Type())) {
    stack.SetError();
    return;
  }
  MergeStack(stack);
}

void Parser::ProcessBinaryConnective(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token) {
  (void)tokenizer;

  if (stack.empty()) {
    stack.SetError();
    return;
  }

  enum Expr::Type type = kBinaryAllToType.at(token);
  stack.top()->Append(type);
}

void Parser::ProcessUnaryProp(ExprStack &stack, Tokenizer &tokenizer,
                              Token &token) {
  (void)tokenizer;
  enum Expr::Type type = kUnaryPropToType.at(token);
  stack.emplace(std::make_shared<UnaryExpr>(type));
}

void Parser::ProcessLiteralProp(ExprStack &stack, Tokenizer &tokenizer,
                                Token &token) {
  (void)tokenizer;
  stack.emplace(std::make_shared<Literal>(std::move(token)));
  MergeStack(stack);
}

void Parser::ProcessUnaryPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                   Token &token) {
  (void)tokenizer;
  Token next_token = tokenizer.PeekToken();
  tokenizer.PopToken();

  if (std::find(kVarPredicate.begin(), kVarPredicate.end(), next_token) ==
      kVarPredicate.end()) {
    stack.SetError();
    return;
  }

  stack.emplace(std::make_shared<QuantifiedUnaryExpr>(
      kUnaryPredicateToType.at(token), std::move(next_token)));
}

// Process formulas like P(x,y)
void Parser::ProcessLiteralPredicate(ExprStack &stack, Tokenizer &tokenizer,
                                     Token &token) {
  (void)tokenizer;
  std::vector<Token> token_holder;
  token_holder.reserve(5);

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

  Tokenizer tokenizer{std::move(line)};
  ExprStack stack;

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

  if (stack.Error() || stack.size() != 1 || (proposition == predicate) ||
      !stack.top()->Complete() || stack.top()->Error()) {
    return ParserOutput{Formula{}, ParseResult::kNotAFormula};
  }

  return ParserOutput{Formula{std::move(stack.top())},
                      proposition ? ParseResult::kProposition
                                  : ParseResult::kPredicate};
}