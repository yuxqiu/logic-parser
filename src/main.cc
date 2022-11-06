#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "parser.hh"
#include "tableau.hh"

void PrintParserInformation(std::ostream &out,
                            const Parser::ParserOutput &parser_out) {
  const Parser::ParseResult result = parser_out.Result();
  const std::string &line = parser_out.Formula().Description();
  const Formula &formula = parser_out.Formula();

  if (result == Parser::ParseResult::kNotAFormula) {
    out << parser_out.RawFormula() << " is not a formula." << std::endl;
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
      out << ", its connective is " << Expr::TypeToString(formula.Type());
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
      out << ", its connective is " << Expr::TypeToString(formula.Type());
      out << ", and its right hand side is " << children[1].Description();
      out << "." << std::endl;
      return;
    }
  }
}

void PrintTableauInformation(std::ostream &out,
                             const Parser::ParserOutput &parser_out,
                             const Tableau::TableauResult &tableau_out) {
  const std::string &line = parser_out.Formula().Description();
  switch (tableau_out) {
  case Tableau::TableauResult::kUnsatisfiable:
    out << line << " is not satisfiable." << std::endl;
    break;
  case Tableau::TableauResult::kSatisfiable:
    out << line << " is satisfiable." << std::endl;
    break;
  case Tableau::TableauResult::kUndecidable:
    out << line << " may or may not be satisfiable." << std::endl;
    break;
  }
}

auto main(int argc, char *argv[]) -> int {
  if (argc != 2) {
    std::cout << "Usage: ./" << argv[0] << " filename" << std::endl;
    return 0;
  }

  std::ifstream file{argv[1]};
  if (!file) {
    std::cerr << "Failed to open the file" << std::endl;
    return 1;
  }

  bool parse{false};
  bool solve{false};

  std::string line;
  if (std::getline(file, line)) {
    std::stringstream iss(line);
    std::string word;
    while (iss >> word) {
      if (word == "PARSE") {
        parse = true;
      } else if (word == "SAT") {
        solve = true;
      } else {
        std::cerr << "Unknown Command" << std::endl;
      }
    }
  }

  if (!parse) {
    return 0;
  }

  while (std::getline(file, line)) {
    auto parse_out = Parser::Parse(line);
    PrintParserInformation(std::cout, parse_out);
    if (solve) {
      if (parse_out.Result() == Parser::ParseResult::kNotAFormula) {
        PrintParserInformation(std::cout, parse_out);
        continue;
      }
      auto tableau_result = Tableau::Solve(parse_out);
      PrintTableauInformation(std::cout, parse_out, tableau_result);
    }
  }
  return 0;
}