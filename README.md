# logic-parser

This project implements a one-pass iterative parser and SAT solver for propositional and first-order logic.

## Brief

It can pass the propositional logic that follows the syntax shown below:

$$
\begin{align}
    Formula &:= Prop\\
            &| \; \neg Formula\\
            &| \; (Formula \; * \; Formula)
\end{align}
$$

where Prop can be p, q, r, s, and `*` can be `^/v/>` (and, or, implies). You can easily adjust the symbols and connectives it supports by changing `parser.cc`.

The FOL syntax it supports is similar to the propositional syntax, except that we introduce variables `x, y, z, w`, existential and universal quantifier, and assume that all the predicates are binary.

The exact syntax looks like this:

$$
\begin{align}
    Var &:= x, y, z, w\\
    Pred &:= P, Q, R, S\\
    Connective &:= \&, \^{}, >\\
    Formula &:= Pred(Var, Var)\\
            &| \; \neg Formula\\
            &| \; E \; Formula \; (\exists)\\
            &| \; A \; Formula \; (\forall)\\
            &| \; (Formula \; Connective \; Formula)
\end{align}
$$

For the SAT solver, it's worth noting that when solving the SAT problem of FOL, the solver assumes that there are no free variables in the formula. Also, since the SAT problem of FOL is undecidable, the solver will try to add at most ten constants to the tableau. If it cannot solve the problem after adding these ten constants, it will output "may or may not be satisfiable". This behaviour can be adjusted in `constant.hh` (which manages the constant).


## Iterative Parser

Three main parts need to be implemented iteratively:
- Construct the AST of the formula
- Destruct the AST
- Copy any subtree of the AST

The second task is straightforward. It can be implemented by using a queue.

The first task is also relatively simple. We only need to define the behaviours of our parser when we encounter different categories of characters. You can refer to the implementation in `parser.cc`.

The third task is a bit tedious. The current implementation first flattens the tree into an array and then rebuilds the tree based on this array, which is not ideal in speed and can potentially be improved.


## Test

There are tests provided in the `tests` directory. You can compile the program by using `make release=1`. Then, launch the program by specifying the name of the test file `./bin/release/src/main.out ./tests/{filename}`.


## Contributions

Many things could be improved in this project
- Report user-friendly errors during the parsing phase
- Adopt contract-based design. Specify the pre- and post-condition of each function. By doing so, we can potentially reduce many condition checks that spread across the parser functions
- ...


## License

[MIT LICENSE](./LICENSE)