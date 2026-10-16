# Zenith 言語仕様

Zenith は、前置記法（ポーランド記法）と遅延評価を備えた、静的型付きの純粋関数型言語です。括弧を一切使わず、各名前が「いくつの引数を取るか」（**レベル**）と型の情報から、式の区切り方を決めます。

```
++ :: -> 0 0;
++ : -> int int = \ x : int + x 1;

twice :: -> 1 -> 0 0;
twice<T> : -> -> T T -> T T = \ f : -> T T \ x : T f f x;

// 入力された整数 x に対して x^2 + 2 を返す
main :: -> 0 0;
main : -> int int = \ x : int twice<int> ++ * x x;
```

- `::` は**レベル宣言**です。名前が何個の引数を取り、それぞれの引数が何個の引数を取る値なのかを表します。
- `:` は**定義**です。型と本体を与えます。
- 本体の `twice<int> ++ * x x` は、レベルの情報から `⟦twice⟨int⟩ ++ ⟦* x x⟧⟧` と区切られます（`⟦ ⟧` は区切り方を示すための記法。[01](01-notation.md) の 1.4）。

## 構成

| ファイル | 内容 | 種別 |
|---|---|---|
| [01-notation.md](01-notation.md) | 文法の記法、用語、構文木表記 | 規範 |
| [02-lexical.md](02-lexical.md) | 字句（文字、語、リテラル、名前） | 規範 |
| [03-program.md](03-program.md) | プログラムと文、名前空間、処理の段階 | 規範 |
| [04-levels.md](04-levels.md) | レベル | 規範 |
| [05-types.md](05-types.md) | 型、`::` と `:` の照合、オーバーロード | 規範 |
| [06-expressions.md](06-expressions.md) | 式の形 | 規範 |
| [07-parsing.md](07-parsing.md) | 式の読み方（区切り方の決定） | 規範 |
| [08-type-inference.md](08-type-inference.md) | 型推論とオーバーロードの解決 | 規範 |
| [09-evaluation.md](09-evaluation.md) | 評価 | 規範 |
| [10-builtins.md](10-builtins.md) | 組み込みの名前 | 規範 |
| [11-program-io.md](11-program-io.md) | プログラムの実行と入出力 | 規範 |
| [12-core.md](12-core.md) | Core（推論結果をすべて書き出した形） | 規範 |
| [13-grammar.md](13-grammar.md) | 文法のまとめ | 参考 |
| [appendix/a-examples.md](appendix/a-examples.md) | サンプルの読み方 | 参考 |
| [appendix/b-rationale.md](appendix/b-rationale.md) | 設計の理由 | 参考 |
| [appendix/c-open-issues.md](appendix/c-open-issues.md) | 未決事項と、この文書で補った細部 | 参考 |

- 「規範」の章が言語の定義です。「参考」の章は要約や説明で、規範の章と食い違う場合は規範の章が優先します。
- 規則には `[LEX-3]` のような番号を付けています。接頭辞は章ごとに決まっています（[01](01-notation.md) の 1.5）。

## 最初に読むところ

1. [04-levels.md](04-levels.md)：レベルの考え方
2. [07-parsing.md](07-parsing.md) の 7.1：区切り方を決める原則
3. [appendix/a-examples.md](appendix/a-examples.md)：サンプルが実際にどう読まれるか

## 状態

- 版：2.0 草案
- 最終更新：2026-10-14
- 未決事項：[appendix/c-open-issues.md](appendix/c-open-issues.md)
