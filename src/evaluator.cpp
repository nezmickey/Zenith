#ifndef _EVALUATOR_
#define _EVALUATOR_

#include "include_all"
#include "node.cpp"
#include "value.cpp"

using namespace std;

class Evaluator {
public:
    // 値を強制評価する (Thunkなら中身を計算)
    // Thunk はメモ化済みであれば cached_value を返す。
    static ValuePtr force(ValuePtr v) {
        if (auto thunk = dynamic_pointer_cast<Thunk>(v)) {
            if (thunk->is_evaluated) {
                return thunk->cached_value;
            }
            // 評価してキャッシュに保存
            thunk->cached_value = force(evaluate(thunk->node, thunk->env));
            thunk->is_evaluated = true;
            return thunk->cached_value;
        }
        return v;
    }

    static ValuePtr evaluate(NodePtr node, EnvPtr env) {
        if (auto intNode = dynamic_pointer_cast<IntLit>(node)) {
            return make_shared<IntValue>(intNode->value);
        }
        else if (auto boolNode = dynamic_pointer_cast<BoolLit>(node)) {
            return make_shared<BoolValue>(boolNode->value);
        }
        else if (auto idNode = dynamic_pointer_cast<Identifier>(node)) {
            return env->get(idNode->name);
        }
        else if (auto lamNode = dynamic_pointer_cast<Lambda>(node)) {
            // 関数定義: クロージャを生成
            return make_shared<FunctionValue>(lamNode->param, lamNode->body, env);
        }
        else if (auto appNode = dynamic_pointer_cast<Application>(node)) {
            // 関数適用
            ValuePtr funcVal = force(evaluate(appNode->func, env)); // 関数自体は評価する

            // 引数は遅延評価のためThunkとして渡す
            ValuePtr argThunk = make_shared<Thunk>(appNode->arg, env);

            if (auto fn = dynamic_pointer_cast<FunctionValue>(funcVal)) {
                // ユーザー定義関数: 新しい環境を作って実行
                EnvPtr newEnv = make_shared<Env>(fn->env);
                newEnv->define(fn->param_name, argThunk);
                return evaluate(fn->body, newEnv);
            }
            else if (auto prim = dynamic_pointer_cast<PrimitiveFunctionValue>(funcVal)) {
                // プリミティブ関数
                return prim->func(argThunk);
            }
            else {
                throw runtime_error("Attempt to call a non-function value: " + funcVal->to_string());
            }
        }
        throw runtime_error("Unknown node type");
    }
};

#endif