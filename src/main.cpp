#include "include_all"
#include <fstream>
using namespace std;

#include "token.cpp"
#include "lexer.cpp"
#include "type.cpp"
#include "node.cpp"
#include "parser.cpp"
#include "evaluator.cpp"
#include "value.cpp"

// --- 組み込み演算子ファクトリ ---

// 二項整数演算: int -> int -> int
ValuePtr create_binary_op(function<int(int, int)> op) {
    return make_shared<PrimitiveFunctionValue>([op](ValuePtr x_thunk) -> ValuePtr {
        return make_shared<PrimitiveFunctionValue>([op, x_thunk](ValuePtr y_thunk) -> ValuePtr {
            auto x_val = dynamic_pointer_cast<IntValue>(Evaluator::force(x_thunk));
            auto y_val = dynamic_pointer_cast<IntValue>(Evaluator::force(y_thunk));
            return make_shared<IntValue>(op(x_val->value, y_val->value));
        });
    });
}

// 比較演算子: int -> int -> bool
ValuePtr create_comparison_op(function<bool(int, int)> op) {
    return make_shared<PrimitiveFunctionValue>([op](ValuePtr x_thunk) -> ValuePtr {
        return make_shared<PrimitiveFunctionValue>([op, x_thunk](ValuePtr y_thunk) -> ValuePtr {
            auto x_val = dynamic_pointer_cast<IntValue>(Evaluator::force(x_thunk));
            auto y_val = dynamic_pointer_cast<IntValue>(Evaluator::force(y_thunk));
            return make_shared<BoolValue>(op(x_val->value, y_val->value));
        });
    });
}

// if 関数: bool -> int -> int -> int
ValuePtr create_if_op() {
    return make_shared<PrimitiveFunctionValue>([](ValuePtr cond_thunk) -> ValuePtr {
        return make_shared<PrimitiveFunctionValue>([cond_thunk](ValuePtr true_thunk) -> ValuePtr {
            return make_shared<PrimitiveFunctionValue>([cond_thunk, true_thunk](ValuePtr false_thunk) -> ValuePtr {
                auto cond_val = dynamic_pointer_cast<BoolValue>(Evaluator::force(cond_thunk));
                if (cond_val->value) {
                    return Evaluator::force(true_thunk);
                } else {
                    return Evaluator::force(false_thunk);
                }
            });
        });
    });
}

// --- 組み込み関数の一元登録 ---
// Parser への型登録と globalEnv への値登録を一か所にまとめる。
// 組み込みを追加する際はここだけを変更すればよい。
void setup_builtins(Parser& parser, EnvPtr globalEnv) {
    TypePtr intT  = Type::getInt();
    TypePtr boolT = Type::getBool();
    TypePtr binOpT  = Type::createCurriedFunc({intT, intT}, intT);
    TypePtr compOpT = Type::createCurriedFunc({intT, intT}, boolT);
    TypePtr ifT     = Type::createCurriedFunc({boolT, intT, intT}, intT);

    // 算術演算子
    parser.register_builtin("+", binOpT);
    parser.register_builtin("-", binOpT);
    parser.register_builtin("*", binOpT);
    parser.register_builtin("/", binOpT);
    parser.register_builtin("%", binOpT);
    globalEnv->define("+", create_binary_op([](int a, int b){ return a + b; }));
    globalEnv->define("-", create_binary_op([](int a, int b){ return a - b; }));
    globalEnv->define("*", create_binary_op([](int a, int b){ return a * b; }));
    globalEnv->define("/", create_binary_op([](int a, int b){ return a / b; }));
    globalEnv->define("%", create_binary_op([](int a, int b){ return a % b; }));

    // 比較演算子
    parser.register_builtin("==", compOpT);
    parser.register_builtin("<",  compOpT);
    parser.register_builtin(">",  compOpT);
    parser.register_builtin(">=", compOpT);
    parser.register_builtin("<=", compOpT);
    parser.register_builtin("!=", compOpT);
    globalEnv->define("==", create_comparison_op([](int a, int b){ return a == b; }));
    globalEnv->define("<",  create_comparison_op([](int a, int b){ return a <  b; }));
    globalEnv->define(">",  create_comparison_op([](int a, int b){ return a >  b; }));
    globalEnv->define(">=", create_comparison_op([](int a, int b){ return a >= b; }));
    globalEnv->define("<=", create_comparison_op([](int a, int b){ return a <= b; }));
    globalEnv->define("!=", create_comparison_op([](int a, int b){ return a != b; }));

    // 制御構造
    parser.register_builtin("if", ifT);
    globalEnv->define("if", create_if_op());
}

string get_code(int argc, char* argv[]){
    if(argc > 2){
        cerr << "Usage: ./zenith [filename]" << endl;
        return "";
    }
    string filename = argv[1];

    ifstream ifs(filename);
    if(!ifs){
        cerr << "Error: ファイルを開けません: " << filename << endl;
        return "";
    }
    return string(
        (istreambuf_iterator<char>(ifs)),
        istreambuf_iterator<char>()
    );
}

int main(int argc, char* argv[]){
    string code = get_code(argc, argv);

    Lexer lexer;
    vector<Token> tokens = lexer.tokenize(code);

    // 実行環境の構築 (パース前に生成し、組み込み登録で共有する)
    EnvPtr globalEnv = make_shared<Env>();

    Parser parser(tokens);
    setup_builtins(parser, globalEnv);

    try {
        parser.parse_program();

        // パース結果をトップレベル定義として遅延登録
        for(auto const& [name, node] : parser.get_nodes()) {
            globalEnv->define(name, make_shared<Thunk>(node, globalEnv));
        }

        // main の実行
        ValuePtr result = Evaluator::force(globalEnv->get("main"));
        cout << "Result: " << result->to_string() << endl;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}