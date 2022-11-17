#pragma once

#include <memory>
#include <string>
#include <vector>

#include "constant.hh"
#include "tokenizer.hh"

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

  [[nodiscard]] auto Type() const -> Type;

  // Helper function to deal with Expr::Type
  [[nodiscard]] static auto IsLiteral(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto IsUnary(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto IsBinary(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto Negate(enum Expr::Type type) -> enum Expr::Type;

  static auto Description(const Expr *expr, std::string &out, uint64_t num)
      -> void;
  static auto TypeToString(enum Type type) -> std::string;

  // Get/Set error during constructing stage
  [[nodiscard]] auto Error() const -> bool;
  auto SetError() -> void;

  [[nodiscard]] auto ChildrenSize() const -> uint64_t;

  explicit Expr() = default;
  explicit Expr(enum Type type);

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

  // View the children of the current node (shared ownership)
  [[nodiscard]] virtual auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> = 0;

  // Get Token of the Expr (if there is any)
  //
  // A token is
  //  - Literal (p or P(x, y))
  //  - Quantified Var
  [[nodiscard]] virtual auto Infos() const -> std::vector<Token> = 0;

protected:
  enum Type type_ {
    Type::kNull
  }; // protected because BinaryExpr needs to set type_ later

private:
  bool error_{false};

  friend class Formula;
};