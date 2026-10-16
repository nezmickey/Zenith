# 13. 文法のまとめ（参考）

各章の文法を1か所に集めたものです。規範は各章の本文で、この章と食い違う場合は各章が優先します。

## 13.1 字句（[02](02-lexical.md)）

```
sign           ::= "+" | "-"
digits         ::= [0-9]+
exponent       ::= ( "e" | "E" ) sign? digits
int-literal    ::= sign? digits
double-literal ::= sign? digits ( "." digits exponent? | exponent )
string-literal ::= '"' ( 文字 | '\"' | '\\' | '\n' | '\t' )* '"'
bool-literal   ::= "true" | "false"
literal        ::= int-literal | double-literal | string-literal | bool-literal

name-word      ::= NAME level-region? type-region?
level-region   ::= "[" ( NAT | LVAR )* "]"
type-region    ::= "<" type+ ">"
```

| 項目 | 規則 |
|---|---|
| 空白 | スペース、タブ、CR、LF だけ（[LEX-2]） |
| 区切り文字 | `;`（[LEX-3]） |
| 予約文字 | `[` `]`（[LEX-2]） |
| コメント | 語の先頭の `//`（行末まで）と `/*`（`*/` まで、入れ子なし）（[LEX-4]） |
| 予約語 | `type` `match` `\` `=` `->` `:` `::` `true` `false` `int` `double` `bool` `string`（[LEX-6]） |
| 文脈予約語 | 型宣言の中の `\|`（[LEX-6]） |
| 数値のように見える語 | リテラルでなければエラー（[LEX-7]） |
| 名前の範囲 | 宣言の表を使って決める（[LEX-18]〜[LEX-20]） |
| 型の位置 | 名前の表を使わずに読む（[LEX-21]） |

## 13.2 プログラム（[03](03-program.md)）

```
program    ::= stmt*
stmt       ::= type-decl | level-decl | def
type-decl  ::= "type" TNAME ( "<" TVAR+ ">" )? "=" alt+ ";"
alt        ::= "|" CTOR type*
level-decl ::= NAME ( "[" LVAR* "]" )? "::" level ";"
def        ::= NAME ( "[" LVAR* "]" )? ( "<" TVAR+ ">" )? ":" type "=" expr ";"
```

## 13.3 レベル（[04](04-levels.md)）

```
level      ::= "->" arg-level level | ret-level
arg-level  ::= level | NAT1
ret-level  ::= "0" | LVAR
```

## 13.4 型（[05](05-types.md)）

```
type   ::= "->" type type | tatom
tatom  ::= "int" | "double" | "bool" | "string"
         | TNAME ( "<" type+ ">" )?
         | TVAR
```

## 13.5 式（[06](06-expressions.md)）

```
expr    ::= head arg*
arg     ::= expr
head    ::= literal | ref | lambda | match
ref     ::= NAME ( "[" larg* "]" )? ( "<" type+ ">" )?
larg    ::= NAT | LVAR
lambda  ::= "\" ( "[" larg "]" )? BINDER ":" type expr
match   ::= "match" ( "[" larg "]" )? expr branch+
branch  ::= CTOR BINDER{n} expr          -- n はそのコンストラクタのフィールドの数
```

## 13.6 区切り方の規則の要約（[07](07-parsing.md)）

**適用**（[PAR-4]）：頭のレベル (kh, th)、期待レベル (k, t) について、上から順に調べる。

| 条件 | 動作 |
|---|---|
| t が不明 | 止まる |
| th と t が分かっていて異なる | エラー |
| kh > k | 引数を1つ読んで繰り返す |
| kh = k | 止まる |
| th が不明 | 止まる |
| それ以外 | エラー |

**引数の期待**（[PAR-6]）：関数はレベル宣言の矢印を使い切るまではレベル宣言から、それ以外は型から。

**ラムダ**（[PAR-7]）：

| 期待レベル | 扱い | 本体のレベル |
|---|---|---|
| (0, t)、t が分かっている | その場で適用 | (0, t) |
| (k′ ≥ 1, t) | 関数 | (k′ − 1, t) |
| (0, ?) | 関数 | (0, ?) |

`[k]` があれば本体のレベルを置き換える（矛盾すればエラー）。なければ、末尾が不明な本体のレベルは末尾を 0 とする。

**match**（[PAR-8]）：対象をレベル0で読み、最初のコンストラクタから型を決め、そのコンストラクタの数だけ分岐を読む。本体のレベルはラムダと同じ方法で決める。

## 13.7 Core（[12](12-core.md)）

```
core-program   ::= core-stmt*
core-stmt      ::= core-type-decl | core-def
core-type-decl ::= "type" TNAME ( "⟨" TVAR+ "⟩" )? "=" ( "|" CTOR ctype* )+ ";"
core-def       ::= NAME ( "⟨" TVAR+ "⟩" )? ":" ctype "=" cexpr ";"

ctype   ::= "int" | "double" | "bool" | "string"
          | TNAME ( "⟨" ctype+ "⟩" )?
          | TVAR
          | "⟦" "->" ctype ctype "⟧"

cexpr   ::= literal
          | VAR
          | GNAME ( "⟨" ctype+ "⟩" )?
          | "⟦" GNAME ":" ctype "⟧"
          | CTOR ( "⟨" ctype+ "⟩" )?
          | "⟦" cexpr cexpr+ "⟧"
          | "⟦" "\" VAR ":" ctype cexpr "⟧"
          | "⟦" "match" cexpr cbranch+ "⟧"
cbranch ::= "⟦" CTOR ( VAR ":" ctype )* "⇒" cexpr "⟧"
```
