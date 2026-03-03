# Zenith
Zenithは**ポーランド記法**・**遅延評価**を備えた**純粋関数言語**です。  
この言語は、**「型情報が文法（構造）を決定する」** という極めて合理的かつノイズの少ない設計が特徴です。  
詳しい言語仕様は`docs/spec.md`にまとめてあります。

## ビルド・実行方法
Windows向けの方法を書いています。MacやLinuxでは適宜読み替えてください。
### ビルド
直接main.cppをコンパイルしてください。
```terminal
g++ main.cpp -o zenith.exe
```
### 実行
ファイル名を引数にとって実行してください。
```terminal
.\zenith.exe <filename>
```

## サンプルコード
### フィボナッチ数列
```zenith
fibonacci :: int -> int;
fibonacci n = 
    if == n 1 1 
    if == n 2 1 
    + fibonacci - n 1 fibonacci - n 2;
main :: int;
main = fibonacci 10;
```
実行結果：
```
55
```
### 高階関数
`twice`という与えられた関数を2回適用する関数と、`add2`という2を足す関数を定義し、`twice add2 10`を実行する。
他の言語で言うところの`twice(add2, 10)`のように関数を適用している。
```zenith
twice :: (int -> int) -> int -> int;
twice f x = f f x;
add2 :: int -> int;
add2 = + 2;
main :: int;
main = twice add2 10;
```
実行結果：
```
14
```
他のサンプルコードは`samples/`ディレクトリにあります。