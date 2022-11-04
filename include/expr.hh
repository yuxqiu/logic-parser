#pragma once

#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "constant.hh"
#include "tokenizer.hh"

class Expr {
public:
  // The ordering here is important as it helps us sort the Expr
  enum class Type {
    kNull,
    kNeg,       // for any formula
    kAnd,       // alpha formula
    kExist,     // delta formula
    kOr,        // beta formula
    kImpl,      // beta formula
    kUniversal, // gamma formula
    kLiteral,
  };

  [[nodiscard]] auto Type() const -> Type;

  [[nodiscard]] static auto IsLiteral(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto IsUnary(enum Expr::Type type) -> bool;
  [[nodiscard]] static auto IsBinary(enum Expr::Type type) -> bool;

  [[nodiscard]] auto Error() const -> bool;
  void SetError();

  virtual ~Expr() = default;

  virtual void Append(std::shared_ptr<Expr> expr) = 0;
  virtual void Append(enum Type type) = 0;

  // [[nodiscard]] virtual auto Expand(const std::set<Constant> &constants)
  //     -> std::vector<std::vector<Formula>> = 0;

  [[nodiscard]] virtual auto Complete() const -> bool = 0;

  [[nodiscard]] virtual auto Description() const -> std::string = 0;

protected:
  enum Type type_ { Type::kNull };
  bool error_{false};

private:
  [[nodiscard]] virtual auto TakeChildren()
      -> std::vector<std::shared_ptr<Expr>> = 0;
  [[nodiscard]] virtual auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> = 0;

  friend auto operator<(const Expr &lhs, const Expr &rhs) -> bool;

  friend class FormulaViewer;
  friend class FormulaOwner;
};

auto operator<(const Expr &lhs, const Expr &rhs) -> bool;
auto operator<<(std::ostream &out, enum Expr::Type type) -> std::ostream &;

// A Special Literal -p where p is literal
// Need to handle this case - Use decorator later!
class Literal : public Expr {
public:
  explicit Literal(Token val);
  ~Literal() override = default;

  void Append(std::shared_ptr<Expr> expr) override;
  void Append(enum Type type) override;

  [[nodiscard]] auto Complete() const -> bool override;

  // [[nodiscard]] auto Expand(const std::set<Constant> &constants)
  //     -> std::vector<std::vector<Formula>> override;

  [[nodiscard]] auto Description() const -> std::string override;

private:
  [[nodiscard]] auto TakeChildren()
      -> std::vector<std::shared_ptr<Expr>> override;
  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> override;

  Token val_;
};

class PredicateLiteral : public Literal {
public:
  explicit PredicateLiteral(Token val, Token left_var_, Token right_var_);
  ~PredicateLiteral() override = default;

  [[nodiscard]] auto Description() const -> std::string override;

private:
  Token left_var_, right_var_;
};

class UnaryExpr : public Expr {
public:
  explicit UnaryExpr(enum Type type);
  ~UnaryExpr() override = default;

  void Append(std::shared_ptr<Expr> expr) override;
  void Append(enum Type type) override;

  [[nodiscard]] auto Complete() const -> bool override;

  // [[nodiscard]] auto Expand(const std::set<Constant> &constants)
  //     -> std::vector<std::vector<Formula>> override;

  [[nodiscard]] auto Description() const -> std::string override;

private:
  [[nodiscard]] auto TakeChildren()
      -> std::vector<std::shared_ptr<Expr>> override;
  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> override;

  std::shared_ptr<Expr> expr_{};
};

// A Special UnaryExpr Expx where E is a quantifier
// Need to handle this case (by checking an additional Token)
class QuantifiedUnaryExpr : public UnaryExpr {
public:
  explicit QuantifiedUnaryExpr(enum Type type, Token var);
  ~QuantifiedUnaryExpr() override = default;

  // [[nodiscard]] auto Expand(const std::set<Constant> &constants)
  //     -> std::vector<std::vector<Formula>> override;

  [[nodiscard]] auto Description() const -> std::string override;

private:
  Token var_;
};

class BinaryExpr : public Expr {
public:
  ~BinaryExpr() override = default;

  void Append(std::shared_ptr<Expr> expr) override;
  void Append(enum Type type) override;

  [[nodiscard]] auto Complete() const -> bool override;

  // [[nodiscard]] auto Expand(const std::set<Constant> &constants)
  //     -> std::vector<std::vector<Formula>> override;

  [[nodiscard]] auto Description() const -> std::string override;

private:
  [[nodiscard]] auto TakeChildren()
      -> std::vector<std::shared_ptr<Expr>> override;
  [[nodiscard]] auto ViewChildren() const
      -> std::vector<std::shared_ptr<Expr>> override;

  std::shared_ptr<Expr> expr_lhs_{}, expr_rhs_{};
};
