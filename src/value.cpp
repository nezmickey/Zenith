#ifndef _VALUE_
#define _VALUE_

#include "include_all"
#include "node.cpp" // ASTノードの定義が必要
#include <functional>

using namespace std;

// 前方宣言
struct Env;
struct Value;

using EnvPtr = shared_ptr<Env>;
using ValuePtr = shared_ptr<Value>;

// --- 環境 (Environment) ---
// 変数名と値のバインディングを管理する。
// 親環境へのポインタを持つことでスコープチェーンを実現する。
struct Env {
    map<string, ValuePtr> bindings;
    EnvPtr parent;

    Env(EnvPtr p = nullptr) : parent(p) {}

    // 変数の値を検索
    ValuePtr get(const string& name) {
        if(bindings.count(name)) {
            return bindings[name];
        }else if(parent) {
            return parent->get(name);
        }
        throw runtime_error("Undefined variable: " + name);
    }

    // 変数を定義（現在のスコープに）
    void define(const string& name, ValuePtr val) {
        bindings[name] = val;
    }
};

// --- 値 (Runtime Value) ---
// 実行時に生成されるオブジェクトの基底クラス
struct Value {
    virtual ~Value() = default;
    virtual string to_string() const = 0;
};

// 整数値
struct IntValue : Value {
    int value;
    IntValue(int v) : value(v) {}
    string to_string() const override { return std::to_string(value); }
};

// 真偽値
struct BoolValue : Value {
    bool value;
    BoolValue(bool v) : value(v) {}
    string to_string() const override { return value ? "true" : "false"; }
};

// 関数値 (クロージャ)
// Zenithはカリー化されているため、関数は「引数名」と「本体」と「定義時の環境」を持つ。
// 実行されると、環境に引数をバインドして本体を評価する。
struct FunctionValue : Value {
    string param_name; // 受け取る引数の名前
    NodePtr body;      // 関数本体のAST
    EnvPtr env;        // 定義時の環境 (キャプチャされた変数など)

    FunctionValue(string param, NodePtr b, EnvPtr e) 
        : param_name(param), body(b), env(e) {}

    string to_string() const override { return "<function " + param_name + ">"; }
};

// サンク (Thunk) - 遅延評価用
// 「まだ評価されていない式」を表す。
// 必要になった時点で `node` を `env` の下で評価する。
// 評価結果はメモ化され、同じ Thunk の再評価を防ぐ。
struct Thunk : Value {
    NodePtr node;           // 未評価の式
    EnvPtr env;             // 式が定義された環境
    bool is_evaluated = false;    // メモ化フラグ
    ValuePtr cached_value;        // メモ化済みの値

    Thunk(NodePtr n, EnvPtr e) : node(n), env(e) {}

    string to_string() const override { 
        return "<thunk>";
    }
};

// プリミティブ関数 (組み込み演算子用)
struct PrimitiveFunctionValue : Value {
    function<ValuePtr(ValuePtr)> func;
    PrimitiveFunctionValue(function<ValuePtr(ValuePtr)> f) : func(f) {}
    string to_string() const override { return "<primitive>"; }
};

#endif