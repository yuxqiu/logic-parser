# logic-parser - v2-value

This branch attempts to replace virtual function calls by using `std::variant`.
Compared to the `main` branch, the parser and SAT solver for this branch are slower.
The main reason for this is the number of dynamic memory allocation it makes:
using `std::variant` increases the amount of memory required, as we need to allocate enough memory to accommodate the largest member of the variant (i.e. `BinaryExpr`).
This resulted in significant performance degradation, especially given that I mainly used `tests/large.txt` (containing only unary and literal expression) as input to the benchmark.

A potential improvement would be to define `Expr` as a variant of a shared pointer. This could reduce memory waste. Also, inside each expression, we can use `Expr` directly instead of using a shared pointer to Expr.

## Contributions

Many things could be improved in this project
- Report user-friendly errors during the parsing phase
- Adopt contract-based design. Specify the pre- and post-condition of each function. By doing so, we can potentially reduce many condition checks that spread across the parser functions
- ...


## License

[MIT LICENSE](./LICENSE)