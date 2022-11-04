#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "expr.hh"
#include "formula.hh"

// Viewer is subject to changes if the owner de-constructs before the viewer
class FormulaViewer {
public:
  explicit FormulaViewer(Expr *expr);

  [[nodiscard]] auto Type() const -> enum Expr::Type;

  // Description of the entire formula
  [[nodiscard]] auto Description() const -> std::string;

  [[nodiscard]] auto ViewChildren() const -> std::vector<FormulaViewer>;

protected:
  FormulaViewer() = default;
  Expr *expr_{};
};

class FormulaOwner {
public:
  FormulaOwner() = default;
  explicit FormulaOwner(std::shared_ptr<Expr> &&expr);

  FormulaOwner(const FormulaOwner &) = delete;
  auto operator=(const FormulaOwner &) -> FormulaOwner & = delete;

  FormulaOwner(FormulaOwner &&owner) noexcept = default;
  auto operator=(FormulaOwner &&) noexcept -> FormulaOwner &;

  ~FormulaOwner();

  explicit operator FormulaViewer() const;

private:
  void ReleaseResources();

  std::shared_ptr<Expr> expr_;
};