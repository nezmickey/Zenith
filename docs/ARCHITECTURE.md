# Zenith Architecture Diagram

この図は、Zenith プログラミング言語インタプリタの全体構成と、ソースコードが実行されるまでの流れを示しています。

## 1. コンポーネント関連図 (Class Diagram)

```mermaid
classDiagram
    class Main {
        +get_code()
        +setup_builtins()
        +main()
    }
    class Lexer {
        +tokenize(string) vector~Token~
    }
    class Parser {
        -global_types_
        -global_nodes_
        +parse_program()
        +parse_expr()
        +register_builtin()
    }
    class Evaluator {
        +static evaluate(Node, Env) Value
        +static force(Value) Value
    }
    class Node {
        <<abstract>>
        +TypePtr type
    }
    class Value {
        <<abstract>>
        +to_string()
    }
    class Env {
        -bindings
        -parent
        +get(name)
        +define(name, value)
    }

    Main --> Lexer : uses
    Main --> Parser : uses
    Main --> Evaluator : uses
    Lexer ..> Token : creates
    Parser ..> Token : consumes
    Parser ..> Node : builds (AST)
    Evaluator ..> Node : traverses
    Evaluator ..> Value : produces/consumes
    Evaluator --> Env : manages
    Value <|-- Thunk : lazy evaluation
    Thunk --> Node : delayed
    Thunk --> Env : context
```

## 2. 実行フロー (Sequence Diagram)

```mermaid
sequenceDiagram
    participant U as User/Source Code
    participant L as Lexer
    participant P as Parser
    participant E as Evaluator
    participant Env as Environment

    U->>L: string (source)
    L->>L: tokenization
    L-->>P: vector<Token>
    
    activate P
    P->>P: parse_program()
    P->>P: Type-driven parsing
    P-->>P: Build AST (Nodes)
    deactivate P

    P-->>E: Root Node (main)
    
    activate E
    E->>Env: get("main")
    Env-->>E: Thunk/Value
    E->>E: Evaluator::force()
    E->>E: Recursive Evaluate AST
    E-->>U: Result (IntValue/BoolValue)
    deactivate E
```

## 3. 処理の特徴
- **型駆動パース**: Parser は `Type` 情報に基づいて、関数適用 (Application) の結合を決定します。
- **遅延評価 (Lazy Evaluation)**: `Evaluator` は値を必要とするまで `Thunk` として保持し、`force` された際に初めて計算・メモ化します。
- **カリー化**: すべての関数は 1 引数の連鎖 (`Lambda` ノードの入れ子) として表現されます。
