# 12. Core

**Core** は、Zenith のプログラムから、区切り方・型推論・オーバーロードの解決の結果をすべて書き出した形です。評価の意味は Core の上で定めます（[09](09-evaluation.md)）。実装の中間表現としても使うことを想定しています。

## 12.1 原則

**[CORE-1]**

1. **文脈自由**：構造は、名前の宣言を知らなくても一意に読めます。
2. **レベルがない**：レベル宣言、レベル引数、`\[k]`、`match[k]` はすべて消えます。レベルは表層の区切り方を決めるためだけの情報で、意味には関わりません。
3. **推論で決まった情報がすべて書かれている**：型引数、束縛変数の型、オーバーロードの選択。
4. **正準形**：同じプログラムは1通りにしか書けません（12.4）。

## 12.2 字句

**[CORE-2]** Zenith の字句（[02](02-lexical.md)）に、次の5つの区切り文字を加えます。これらは空白がなくても前後の語を区切ります。

```
⟦  ⟧  ⟨  ⟩  ⇒
```

これらは ASCII 以外の文字なので、Zenith の名前と衝突しません。Core では `< >` を型引数に使わないので、`<` を含む名前（`a<b`）もそのまま書けます。

## 12.3 文法

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
          | GNAME ( "⟨" ctype+ "⟩" )?            -- オーバーロードのない関数
          | "⟦" GNAME ":" ctype "⟧"              -- オーバーロードの選択
          | CTOR ( "⟨" ctype+ "⟩" )?
          | "⟦" cexpr cexpr+ "⟧"                 -- 適用
          | "⟦" "\" VAR ":" ctype cexpr "⟧"      -- ラムダ
          | "⟦" "match" cexpr cbranch+ "⟧"       -- match
cbranch ::= "⟦" CTOR ( VAR ":" ctype )* "⇒" cexpr "⟧"
```

- `⟦` で始まる形は、その次の語で決まります。`->` なら関数型、`\` ならラムダ、`match` なら match、2番目の語が `:` ならオーバーロードの選択、それ以外は適用です。
- `VAR`（局所変数）・`GNAME`（関数）・`CTOR`（コンストラクタ）はどれも名前で、どれを指すかはスコープで決まります。
- オーバーロードされた関数には、関数ごとに複数の `core-def` があります。

## 12.4 正準形

**[CORE-3]**

1. 適用の頭に適用を置きません。`⟦⟦f x⟧ y⟧` は `⟦f x y⟧` と書きます。
2. match の分岐は、型宣言のコンストラクタの順に並べます。
3. 型パラメータを持つ関数とコンストラクタには、必ず型引数を書きます。型パラメータを持たないものには書きません。
4. 定義が2つ以上ある関数は、必ず選択形 `⟦f : τ⟧` で書きます。τ はその場所での f の型です。[03](03-program.md) の [PRG-7] により、τ をインスタンスとして持つ定義はちょうど1つで、型引数もそこから決まります。それ以外の名前は選択形にしません。
5. 局所変数の名前は、元のプログラムのまま残します。
6. 関数型は常に `⟦-> a b⟧` と書きます。

## 12.5 型付け

**[CORE-4]**

- リテラル・局所変数・適用・ラムダは、単純型付きラムダ計算と同じです。
- `f⟨τ…⟩` の型は、f の定義の型の型パラメータに τ… を代入したものです。`C⟨τ…⟩` も同じです。
- `⟦f : τ⟧` では、τ をインスタンスとして持つ f の定義がちょうど1つなければなりません。その型は τ です。
- match では、対象の型が `D⟨τ…⟩` で、分岐は D のコンストラクタを宣言順に1回ずつ持ちます。束縛変数の型は、フィールドの型に τ… を代入したものと等しくなければなりません。すべての分岐の本体は同じ型を持ちます。
- 定義の本体の型は、宣言された型と等しくなければなりません。
- レベルの検査はありません。そのため、Core で書けるプログラムは Zenith より広くなります（たとえば `twice⟨⟦-> int int⟧⟩` は Core では正しいが、Zenith からは作られない）。

## 12.6 Zenith からの変換

| Zenith | Core |
|---|---|
| `type D<Ts> = …;` | `type D⟨Ts⟩ = …;`（フィールドの型は Core の型） |
| `f[…] :: ℓ;` | 消える |
| `f[Ls]<Ts> : σ = e;` | `f⟨Ts⟩ : σ′ = e′;` |
| 関数の参照 `f[ℓs]<τs>` | `f⟨τs′⟩`（レベル引数は消え、型引数は推論の結果で補う） |
| オーバーロードされた関数の参照 | `⟦f : τ⟧` |
| コンストラクタ `C` / `C<τs>` | `C⟨τs′⟩` |
| `\[k] x : τ e` | `⟦\ x : τ′ e′⟧` |
| `match[k] s br…` | `⟦match s′ ⟦C x : τ … ⇒ e′⟧ …⟧`（宣言順） |
| 適用 | `⟦h a1 … an⟧` |
| リテラル | 同じ |

構文木表記（[01](01-notation.md) の 1.4）は、Core の文法を次の点でゆるめたものです。

- 名前は書かれたまま（`[ ]` を残してよく、型引数や選択形を省いてよい）
- 束縛変数の型を省いてよい
- 分岐は書かれた順のまま
- 適用の省略形は任意

## 12.7 例

### twice.zn

```
++ : ⟦-> int int⟧ =
  ⟦\ x : int ⟦⟦+ : ⟦-> int ⟦-> int int⟧⟧⟧ x 1⟧⟧;
twice⟨T⟩ : ⟦-> ⟦-> T T⟧ ⟦-> T T⟧⟧ =
  ⟦\ f : ⟦-> T T⟧ ⟦\ x : T ⟦f ⟦f x⟧⟧⟧⟧;
main : ⟦-> int int⟧ =
  ⟦\ x : int ⟦twice⟨int⟩ ++ ⟦⟦* : ⟦-> int ⟦-> int int⟧⟧⟧ x x⟧⟧⟧;
```

### list.zn

head の `[N]` と `error[N]` は消え、型引数 `⟨T⟩` だけが残ります。

```
type list⟨T⟩ = | Nil | Cons T list⟨T⟩;
head⟨T⟩ : ⟦-> list⟨T⟩ T⟧ =
  ⟦\ xs : list⟨T⟩
    ⟦match xs
      ⟦Nil ⇒ ⟦error⟨T⟩ "head: 空リスト"⟧⟧
      ⟦Cons x : T _ : list⟨T⟩ ⇒ x⟧⟧⟧;
sum : ⟦-> list⟨int⟩ int⟧ =
  ⟦\ xs : list⟨int⟩
    ⟦match xs
      ⟦Nil ⇒ 0⟧
      ⟦Cons x : int rest : list⟨int⟩ ⇒ ⟦⟦+ : ⟦-> int ⟦-> int int⟧⟧⟧ x ⟦sum rest⟧⟧⟧⟧⟧;
```

### local_val.zn

ローカル変数は、その場で適用されるラムダとしてそのまま残ります。

```
main : ⟦-> int int⟧ =
  ⟦\ x : int
    ⟦⟦\ sq : int ⟦⟦* : ⟦-> int ⟦-> int int⟧⟧⟧ sq sq⟧⟧
     ⟦⟦* : ⟦-> int ⟦-> int int⟧⟧⟧ x x⟧⟧⟧;
```

### if[1]

Core では `[1]` が消え、`if` は4つの引数を1つの適用として受け取ります。

```
-- : ⟦-> int int⟧ =
  ⟦\ x : int ⟦⟦- : ⟦-> int ⟦-> int int⟧⟧⟧ x 1⟧⟧;
ap : ⟦-> int int⟧ =
  ⟦\ x : int ⟦if⟨⟦-> int int⟧⟩ ⟦⟦== : ⟦-> int ⟦-> int bool⟧⟧⟧ x 0⟧ ++ -- x⟧⟧;
```

### 型引数の補完と分岐の並べ替え

Zenith のソース（分岐を Cons、Nil の順に書いたもの）：

```
length :: -> 0 0;
length<T> : -> list<T> int = \ xs : list<T>
    match xs Cons y rest + 1 length rest Nil 0;
count1 :: -> 0 0;
count1 : -> int int = \ n : int length Cons \ x : int + x n Nil;
```

Core：

```
length⟨T⟩ : ⟦-> list⟨T⟩ int⟧ =
  ⟦\ xs : list⟨T⟩
    ⟦match xs
      ⟦Nil ⇒ 0⟧
      ⟦Cons y : T rest : list⟨T⟩ ⇒ ⟦⟦+ : ⟦-> int ⟦-> int int⟧⟧⟧ 1 ⟦length⟨T⟩ rest⟧⟧⟧⟧⟧;
count1 : ⟦-> int int⟧ =
  ⟦\ n : int
    ⟦length⟨⟦-> int int⟧⟩
      ⟦Cons⟨⟦-> int int⟧⟩ ⟦\ x : int ⟦⟦+ : ⟦-> int ⟦-> int int⟧⟧⟧ x n⟧⟧ Nil⟨⟦-> int int⟧⟩⟧⟧⟧;
```

### convert_funcs.zn

オーバーロードの選択が2段になる例です。

```
main : ⟦-> int double⟧ =
  ⟦\ x : int ⟦⟦/ : ⟦-> double ⟦-> double double⟧⟧⟧ ⟦⟦to_double : ⟦-> int double⟧⟧ x⟧ 5.0⟧⟧;
```

Core は算術のオーバーロードの選択でかなり長くなります。人が読むためではなく、仕様と実装のための形です。
