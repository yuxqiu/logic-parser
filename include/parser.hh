#pragma once

#include <memory>

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

    [[nodiscard]] auto Formula() -> class Formula & { return formula_; }
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
};