#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "parser.hh"
#include "tableau.hh"

auto PrintParserInformation(std::ostream &out,
                            const Parser::ParserOutput &parser_out) -> void {
  const Parser::ParseResult result = parser_out.Result();
  const Formula &formula = parser_out.Formula();

  if (result == Parser::ParseResult::kNotAFormula) {
    out << parser_out.RawFormula() << " is not a formula.\n";
    return;
  }

  const std::string &line = parser_out.Formula().Description();
  if (result == Parser::ParseResult::kProposition) {
    if (Expr::IsLiteral(formula.Type())) {
      out << line << " is a proposition.\n";
      return;
    }
    if (Expr::IsUnary(formula.Type())) {
      out << line << " is a negation of a propositional formula.\n";
      return;
    }
    if (Expr::IsBinary(formula.Type())) {
      out << line << " is a binary connective propositional formula. ";
      std::vector children = formula.ViewChildren();
      assert(children.size() == 2);
      out << "Its left hand side is " << children[0].Description();
      out << ", its connective is " << formula.Connective();
      out << ", and its right hand side is " << children[1].Description();
      out << ".\n";
      return;
    }
  }

  if (result == Parser::ParseResult::kPredicate) {
    if (Expr::IsLiteral(formula.Type())) {
      out << line << " is an atom.\n";
      return;
    }
    if (formula.Type() == Expr::Type::kNeg) {
      out << line << " is a negation of a first order logic formula.\n";
      return;
    }
    if (formula.Type() == Expr::Type::kUniversal) {
      out << line << " is a universally quantified formula.\n";
      return;
    }
    if (formula.Type() == Expr::Type::kExist) {
      out << line << " is an existentially quantified formula.\n";
      return;
    }
    if (Expr::IsBinary(formula.Type())) {
      out << line << " is a binary connective first order formula. ";
      std::vector children = formula.ViewChildren();
      assert(children.size() == 2);
      out << "Its left hand side is " << children[0].Description();
      out << ", its connective is " << formula.Connective();
      out << ", and its right hand side is " << children[1].Description();
      out << ".\n";
      return;
    }
  }
}

auto PrintTableauInformation(std::ostream &out,
                             const Parser::ParserOutput &parser_out,
                             const Tableau::TableauResult &tableau_out)
    -> void {
  const std::string &line = parser_out.Formula().Description();
  switch (tableau_out) {
  case Tableau::TableauResult::kUnsatisfiable:
    out << line << " is not satisfiable.\n";
    break;
  case Tableau::TableauResult::kSatisfiable:
    out << line << " is satisfiable.\n";
    break;
  case Tableau::TableauResult::kUndecidable:
    out << line << " may or may not be satisfiable.\n";
    break;
  }
}

auto main(int argc, char *argv[]) -> int {
  if (argc != 2) {
    std::cout << "Usage: ./" << argv[0] << " filename\n";
    return 0;
  }

  std::ifstream file{argv[1]};
  if (!file) {
    std::cerr << "Failed to open the file\n";
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
        std::cerr << "Unknown Command\n";
      }
    }
  }

  while (std::getline(file, line)) {
    const auto parse_out = Parser::Parse(line);
    if (parse) {
      PrintParserInformation(std::cout, parse_out);
    }
    if (solve) {
      if (parse_out.Result() == Parser::ParseResult::kNotAFormula) {
        PrintParserInformation(std::cout, parse_out);
        continue;
      }
      const auto tableau_result = Tableau::Solve(parse_out);
      PrintTableauInformation(std::cout, parse_out, tableau_result);
    }
  }
  return 0;
}