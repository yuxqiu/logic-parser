#pragma once

#include <memory>
#include <string>
#include <vector>

#include "constant.hh"
#include "tokenizer.hh"
#include "visitor/expr_visitor.hh"

class Expr {
public:
  // The ordering is used to sort the priority_queue
  enum class Type {
    kNull,
    kLiteral,
    kNeg,       // for any formula
    kAnd,       // alpha formula
    kExist,     // delta formula
    kOr,        // beta formula
    kImpl,      // beta formula
    kUniversal, // gamma formula
  };

  // Helper function to deal with Expr::Type
  [[nodiscard]] static auto IsLiteral(enum Expr::Type type) -> bool {
    return type == Expr::Type::kLiteral;
  }

  [[nodiscard]] static auto IsUnary(enum Expr::Type type) -> bool {
    return type == Expr::Type::kNeg || type == Expr::Type::kExist ||
           type == Expr::Type::kUniversal;
  }

  [[nodiscard]] static auto IsBinary(enum Expr::Type type) -> bool {
    return type == Expr::Type::kAnd || type == Expr::Type::kImpl ||
           type == Expr::Type::kOr;
  }

  [[nodiscard]] static auto Negate(enum Expr::Type type) -> enum Expr::Type;

  [[nodiscard]] auto Type() const -> Type { return type_; }
  auto SetType(enum Expr::Type type) -> void { type_ = type; }

  // Get/Set error during constructing stage
  [[nodiscard]] auto Error() const -> bool { return error_; }
  auto SetError() -> void { error_ = true; }

  explicit Expr() = default;
  explicit Expr(enum Type type) : type_(type) {}

  virtual ~Expr() = default;
  Expr(const Expr &&) = delete;
  Expr(Expr &&) = delete;
  auto operator=(const Expr &) -> Expr & = delete;
  auto operator=(Expr &&) -> Expr & = delete;

  // Append determines which inserted order is correct
  // when we try to construct the expr from raw string
  virtual auto Append(std::shared_ptr<Expr> expr) -> void = 0;
  virtual auto Append(enum Type type) -> void = 0;

  // Whether of not the Expr is completely built
  [[nodiscard]] virtual auto Complete() const -> bool = 0;

  virtual auto Accept(ExprVisitor &visitor) const -> void = 0;

private:
  enum Type type_ { Type::kNull };
  bool error_{false};

  friend class Formula;
};