#pragma once

struct Literal;
struct PredicateLiteral;
struct UnaryExpr;
struct QuantifiedUnaryExpr;
struct BinaryExpr;

class ExprVisitor {
public:
  virtual void Visit(const Literal &literal) = 0;
  virtual void Visit(const PredicateLiteral &predicate) = 0;
  virtual void Visit(const UnaryExpr &unary) = 0;
  virtual void Visit(const QuantifiedUnaryExpr &quantified) = 0;
  virtual void Visit(const BinaryExpr &binary) = 0;

  ExprVisitor() = default;
  virtual ~ExprVisitor() = default;
  ExprVisitor(const ExprVisitor &) = default;
  ExprVisitor(ExprVisitor &&) = default;
  auto operator=(const ExprVisitor &) -> ExprVisitor & = default;
  auto operator=(ExprVisitor &&) -> ExprVisitor & = default;
};