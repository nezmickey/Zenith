# 11. プログラムの実行と入出力

## 11.1 実行のしかた

```
zenith [インタプリタのオプション] ファイル [main の引数 …]
```

**[IO-1]** ファイル名より後ろにある引数は、すべて main に渡します。`-0.01` のように `-` で始まる値も、インタプリタのオプションとしては扱いません。

## 11.2 main の型

**[IO-2]**

- プログラムを実行するには、main の定義がちょうど1つ必要です（main はオーバーロードできません）。
- main は、レベルパラメータも型パラメータも持てません。
- main の型は `-> T1 -> … -> Tn R`（n ≥ 0）で、T1 … Tn と R はどれも基本型（int、double、bool、string）です。n = 0 なら、main の型は R で、レベル宣言は `main :: 0;` です。
- それ以外の型（代数的データ型、関数型）を使った main はエラーです。

```
main :: -> 0 -> 0 -> 0 0;
main : -> int -> double -> string string = \ n : int \ r : double \ s : string …;
```

## 11.3 引数

**[IO-3]**

- main に渡すコマンドラインの引数の数は、n でなければなりません。違えば、評価を始める前にエラーにします。
- 各引数は、対応する Ti に従って変換します。

| Ti | 変換 |
|---|---|
| int | [10](10-builtins.md) の [BLT-5]（to_int）と同じ |
| double | [BLT-6]（文字列からの to_double）と同じ |
| bool | `true` か `false` |
| string | シェルから渡された文字列をそのまま使う。引用符を付けず、`\n` などのエスケープも解釈しない |

- 変換は、遅延評価を待たずに、評価を始める前にすべて行います。失敗したら、評価を始める前にエラーにします。
- 改行などを含む文字列を渡したいときは、シェルの書き方を使います。PowerShell なら `` "a`nb" ``、bash なら `$'a\nb'` です。

## 11.4 出力

**[IO-4]** main の結果を r とします。r が string なら r を、それ以外なら `to_string r`（[BLT-7]）を標準出力に書き、最後に改行（LF）を1つ書きます。string は引用符を付けずにそのまま出力します。

double の桁数や表記を指定したいときは、main の中で `to_fixed` や `to_exp` を使って string を返します。

## 11.5 例

```
main :: -> 0 0;
main : -> int double = \ x : int / to_double x 5.0;
```

```bash
zenith convert_funcs.zn 10
```

出力：

```
2.0
```

文字列を受け取る例：

```
main :: -> 0 -> 0 0;
main : -> int -> string string = \ n : int \ s : string if == n 0 "zero" s;
```

```bash
zenith prog.zn 3 "hello world"
```

出力：

```
hello world
```
