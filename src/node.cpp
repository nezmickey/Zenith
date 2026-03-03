#ifndef _NODE_
#define _NODE_

#include "include_all"
using namespace std;

#include "token.cpp"
#include "type.cpp"

// 前方宣言
struct Node;
using NodePtr = shared_ptr<struct Node>;

// ASTノードの基底クラス
struct Node{
    TypePtr type; // このノードが持つ型 (パース時に決定される)
    virtual ~Node() = default;
    virtual string to_string() const = 0;
};

// 整数リテラル (例: 123)
struct IntLit : Node {
    int value;
    IntLit(int v) : value(v) {
        type = Type::getInt();
    }
    string to_string() const override {
        return std::to_string(value);
    }
};

// 真偽値リテラル (例: true, false)
struct BoolLit : Node {
    bool value;
    BoolLit(bool v) : value(v) {
        type = Type::getBool();
    }
    string to_string() const override {
        return value ? "true" : "false";
    }
};

// 識別子 (例: x, add, +)
struct Identifier : Node {
    string name;
    Identifier(string n, TypePtr t) : name(n) {
        type = t;
    }
    string to_string() const override {
        return name;
    }
};

// 関数適用 (例: f x)
// Zenithはカリー化されているため、複数の引数も ( (f x) y ) のように入れ子のApplicationで表現される
struct Application : Node {
    NodePtr func; // 関数部分
    NodePtr arg;  // 引数部分

    Application(NodePtr f, NodePtr a, TypePtr t) : func(f), arg(a) {
        type = t; // 適用結果の型
    }
    string to_string() const override {
        return "(" + func->to_string() + " " + arg->to_string() + ")";
    }
};

// ラムダ抽象 (関数定義用: \param -> body)
struct Lambda : Node {
    string param;
    NodePtr body;
    Lambda(string p, NodePtr b, TypePtr t) : param(p), body(b) { type = t; }
    string to_string() const override { return "\\" + param + " -> " + body->to_string(); }
};

#endif
