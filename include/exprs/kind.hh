#pragma once

#include <cstdint>

#include "utils/helper.hh"

class ExprKind {
public:
  enum ExprKindInternal : uint8_t {
    kNull,
    kLiteral,
    kNeg,       // for any formula
    kAnd,       // alpha formula
    kExist,     // delta formula
    kOr,        // beta formula
    kImpl,      // beta formula
    kUniversal, // gamma formula
  };

  ExprKind() = default;

  // intentionally ignore explicit because we want ExprKindInternal
  // implicitly convertible to ExprKind
  constexpr ExprKind(ExprKindInternal aFruit) : value_(aFruit) {} // NOLINT

  // Allow switch and comparisons.
  constexpr operator ExprKindInternal() const { return value_; } // NOLINT

  // Prevent usage: if(ExprKind)
  explicit operator bool() const = delete;

  // Helper function to deal with Expr::Type
  [[nodiscard]] static auto IsLiteral(ExprKind type) -> bool {
    return type == ExprKind::kLiteral;
  }

  [[nodiscard]] static auto IsUnary(ExprKind type) -> bool {
    return type == ExprKind::kNeg || type == ExprKind::kExist ||
           type == ExprKind::kUniversal;
  }

  [[nodiscard]] static auto IsBinary(ExprKind type) -> bool {
    return type == ExprKind::kAnd || type == ExprKind::kImpl ||
           type == ExprKind::kOr;
  }

  [[nodiscard]] static auto Negate(ExprKind type) -> ExprKind {
    switch (type) {
    case ExprKind::kAnd:
      return ExprKind::kOr;
    case ExprKind::kExist:
      return ExprKind::kUniversal;
    case ExprKind::kOr:
      [[fallthrough]];
    case ExprKind::kImpl:
      return ExprKind::kAnd;
    case ExprKind::kUniversal:
      return ExprKind::kExist;
    case ExprKind::kNull:
    case ExprKind::kLiteral:
    case ExprKind::kNeg:
      break;
    }
    unreachable();
  }

private:
  ExprKindInternal value_;
};
