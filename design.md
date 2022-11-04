## Interface

### Expression

```
1. Tokenizer
  - Token class
  - give token one by one
    - peek token: optional<string>
    - pop token: void
    - empty: bool
  - In theory, need to implement maximum match, but in our cases, all the token have length 1

2. Expr Interface
  - Enum class: Expr Type
  - is_literal, is_alpha, ...
  - Descendent
    - Non-member
      - operator<<
    - Member
      - shared_ptr to expr
    - public:
      - append(string token): void
      - is_error: bool
      - expandable(set of symbols)
    - private:
      - complete() - whether it's a complete expr now

3. Formula holds Expr
  - managing its destruction (iterative destruct)
  - managing its copy (just copy the shared ptr)
  - no move
  - member
    - shared_ptr to expr
  - expand() return vector of Formula
    - throw runtime_error if called with empty Formula
    - use fair schedule (dequeue gamma)

4. Parser
  - ParsedResult
    - Formula
    - Meta Information, including FOF, Predicate?
  - Parse
    - define Formula Error
    - static member function
    - iteratively on stack
    - return error if failed

4. Theory
  - find first expandable formula (!literal) and expand it (min_heap)
  - keep all the constants (set) when constructing with one formula (which is none)
  - expand() return vector of Theory

5. SAT Solver
```