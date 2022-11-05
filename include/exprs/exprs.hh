#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "tokenizer.hh"

class Expr {
public:
  // The ordering here is important as it helps us sort the Expr
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

  [[nodiscard]] static auto IsLiteral(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto IsUnary(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto IsBinary(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto Negate(enum Expr::Type type) -> enum Expr::Type;

  // 1. Number of branches
  // 2. Number of children
  [[nodiscard]] static auto Expand(const std::shared_ptr<Expr> &expr)
      -> std::vector<std::vector<std::shared_ptr<Expr>>>;

  static void Description(const Expr *expr, std::string &out,
                          uint64_t num);

  [[nodiscard]] auto Error() const -> bool;
  void SetError();

  [[nodiscard]] auto ChildrenSize() const -> uint64_t;

  virtual ~Expr() = default;

  virtual void Append(std::shared_ptr<Expr> expr) = 0;
  virtual void Append(enum Type type) = 0;

  [[nodiscard]] virtual auto Complete() const -> bool = 0;

  [[nodiscard]] virtual auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> = 0;

  [[nodiscard]] virtual auto Infos() const -> std::vector<Token> = 0;

protected:
  enum Type type_ { Type::kNull };
  bool error_{false};

  friend auto operator<(const Expr &lhs, const Expr &rhs) -> bool;
  friend auto operator<<(std::ostream &out, enum Expr::Type type)
      -> std::ostream &;
  friend class Formula;
};

auto operator<(const Expr &lhs, const Expr &rhs) -> bool;
auto operator<<(std::ostream &out, enum Expr::Type type) -> std::ostream &;
