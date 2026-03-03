#ifndef _TYPE_
#define _TYPE_

#include "include_all"
using namespace std;

enum class BaseType { INT, BOOL, FUNCTION };

struct Type;
using TypePtr = shared_ptr<Type>;

struct Type {
    BaseType base;
    virtual ~Type() = default;
    virtual string to_string() const = 0;
    virtual bool equals(const shared_ptr<Type>& other) const = 0;
    
    // ファクトリメソッド
    static TypePtr getInt();
    static TypePtr getBool();
    static TypePtr getFunc(TypePtr param, TypePtr ret);
    
    // パーサー用ヘルパー: リストからカリー化された関数型を構築 (A, B, C -> A -> (B -> C))
    static TypePtr createCurriedFunc(const vector<TypePtr>& params, TypePtr ret);

    static BaseType str_to_basetype(string name){ 
        if(name == "int") return BaseType::INT;
        if(name == "bool") return BaseType::BOOL;
        throw runtime_error("Unknown type: " + name);
    }
};

struct IntType : Type {
    IntType() { base = BaseType::INT; }
    string to_string() const override { return "int"; }
    bool equals(const shared_ptr<Type>& other) const override {
        return other->base == BaseType::INT;
    }
};

struct BoolType : Type {
    BoolType() { base = BaseType::BOOL; }
    string to_string() const override { return "bool"; }
    bool equals(const shared_ptr<Type>& other) const override {
        return other->base == BaseType::BOOL;
    }
};

struct FuncType : Type {
    TypePtr param;
    TypePtr ret;
    FuncType(TypePtr p, TypePtr r) : param(p), ret(r) { base = BaseType::FUNCTION; }
    string to_string() const override { 
        return "(" + param->to_string() + " -> " + ret->to_string() + ")"; 
    }
    bool equals(const shared_ptr<Type>& other) const override {
        if(other->base != BaseType::FUNCTION) return false;
        auto ft = static_pointer_cast<FuncType>(other);
        return param->equals(ft->param) && ret->equals(ft->ret);
    }
};

TypePtr Type::getInt() { return make_shared<IntType>(); }
TypePtr Type::getBool() { return make_shared<BoolType>(); }
TypePtr Type::getFunc(TypePtr param, TypePtr ret) { return make_shared<FuncType>(param, ret); }
TypePtr Type::createCurriedFunc(const vector<TypePtr>& params, TypePtr ret) {
    TypePtr current = ret;
    for(auto it = params.rbegin(); it != params.rend(); it++){
        current = getFunc(*it, current);
    }
    return current;
}

#endif