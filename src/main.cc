#include <fstream>
#include <iostream>
#include <string>

#include "parser.hh"

void PrintInformation(std::ostream &out,
                      const Parser::ParserOutput &parser_out) {
  const Parser::ParseResult result = parser_out.Result();
  const std::string &line = parser_out.RawFormula();
  const Formula &formula = parser_out.Formula();

  if (result == Parser::ParseResult::kNotAFormula) {
    out << line << " is not a formula." << std::endl;
    return;
  }

  if (result == Parser::ParseResult::kProposition) {
    if (Expr::IsLiteral(formula.Type())) {
      out << line << " is a proposition." << std::endl;
      return;
    }
    if (Expr::IsUnary(formula.Type())) {
      out << line << " is a negation of a propositional formula." << std::endl;
      return;
    }
    if (Expr::IsBinary(formula.Type())) {
      out << line << " is a binary connective propositional formula. ";
      std::vector children = formula.ViewChildren();
      assert(children.size() == 2);
      out << "Its left hand side is " << children[0].Description();
      out << ", its connective is " << formula.Type();
      out << ", and its right hand side is " << children[1].Description();
      out << "." << std::endl;
      return;
    }
  }

  if (result == Parser::ParseResult::kPredicate) {
    if (Expr::IsLiteral(formula.Type())) {
      out << line << " is an atom." << std::endl;
      return;
    }
    if (formula.Type() == Expr::Type::kNeg) {
      out << line << " is a negation of a first order logic formula."
          << std::endl;
      return;
    }
    if (formula.Type() == Expr::Type::kUniversal) {
      out << line << " is a universally quantified formula." << std::endl;
      return;
    }
    if (formula.Type() == Expr::Type::kExist) {
      out << line << " is an existentially quantified formula." << std::endl;
      return;
    }
    if (Expr::IsBinary(formula.Type())) {
      out << line << " is a binary connective first order formula. ";
      std::vector children = formula.ViewChildren();
      assert(children.size() == 2);
      out << "Its left hand side is " << children[0].Description();
      out << ", its connective is " << formula.Type();
      out << ", and its right hand side is " << children[1].Description();
      out << "." << std::endl;
      return;
    }
  }
}

auto main() -> int {
  std::ifstream file{"input.txt"};
  if (!file) {
    std::cerr << "Failed to open the file" << std::endl;
    return 1;
  }

  std::string line;
  while (std::getline(file, line)) {
    auto result = Parser::Parse(line);
    PrintInformation(std::cout, result);
  }
  return 0;
}