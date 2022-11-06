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

  static void Description(const Expr *expr, std::string &out, uint64_t num);
  static auto TypeToString(enum Type type) -> std::string;

  // Get/Set error during constructing stage
  [[nodiscard]] auto Error() const -> bool;
  void SetError();

  [[nodiscard]] auto ChildrenSize() const -> uint64_t;

  virtual ~Expr() = default;

  // Append determines which inserted order is correct
  // when we try to construct the expr from raw string
  virtual void Append(std::shared_ptr<Expr> expr) = 0;
  virtual void Append(enum Type type) = 0;

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
  enum Type type_ { Type::kNull };
  bool error_{false};

  friend auto operator<(const Expr &lhs, const Expr &rhs) -> bool;
  friend class Formula;
};

auto operator<(const Expr &lhs, const Expr &rhs) -> bool;
