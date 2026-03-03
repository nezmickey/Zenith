#ifndef _PARSER_
#define _PARSER_

#include "include_all"
#include "token.cpp"
#include "node.cpp"
#include <memory>
#include <stdexcept>
using namespace std;

class Parser{
private:
    const vector<Token>& tokens;
    int index = 0;
    const Token& current_token() const { return tokens[index]; }
    bool isEnd() const { return index >= (int)tokens.size() || current_token().type == TokenType::END; }
    void consume(){ if(!isEnd()) index++; }

    // エラーメッセージに現在のトークンの行・列情報を付加するヘルパー
    runtime_error make_error(const string& msg) const {
        const Token& t = current_token();
        return runtime_error(msg + " (at line " + to_string(t.line) + ", col " + to_string(t.column) + ")");
    }

    void expect(TokenType t){
        if(isEnd() || current_token().type != t)
            throw make_error("Unexpected token: expected '" + current_token().type_to_string(t) + "'");
        consume();
    }

    // 文を一つパース
    // 型定義文または実装の文
    void parse_statement(){
        if(current_token().type != TokenType::IDENTIFIER){
            throw make_error("Expected identifier at start of statement");
        }

        string funcname = current_token().identifier_name;
        consume();
        if(current_token().type == TokenType::DOUBLE_COLON){
            // 型定義のパース: name :: int -> int -> int;
            consume();
            TypePtr functype = parse_type();
            if(global_types_.count(funcname) == 0){
                global_types_[funcname] = functype;
            }else{
                throw make_error("Type of the same function is defined multiple times: " + funcname);
            }
            expect(TokenType::SEMICOLON);
        }else if(current_token().type == TokenType::IDENTIFIER || current_token().type == TokenType::EQUAL){
            // 実装のパース: name param1 param2 = body;
            if(global_types_.count(funcname) == 0)
                throw make_error("Signature not found for: " + funcname);
            TypePtr current_type = global_types_[funcname];
            
            vector<string> args;
            while(current_token().type == TokenType::IDENTIFIER){
                args.push_back(current_token().identifier_name);
                consume();
            }
            expect(TokenType::EQUAL);

            // パラメータの型を解決し、ローカルスコープを作成
            map<string, TypePtr> local_scope;
            for(const string& arg : args){
                if(current_type->base != BaseType::FUNCTION)
                    throw make_error("Too many arguments for function: " + funcname);
                auto ft = static_pointer_cast<FuncType>(current_type);
                local_scope[arg] = ft->param;
                current_type = ft->ret; // 次の型へ
            }

            // 本体のパース (残った型が戻り値の型)
            NodePtr body = parse_expr(current_type, local_scope);
            expect(TokenType::SEMICOLON);

            // Lambdaチェーンの構築 (add x y = body  =>  add = \x -> \y -> body)
            NodePtr def_node = body;
            for(int i = args.size() - 1; i >= 0; --i){
                TypePtr param_type = local_scope[args[i]];
                TypePtr lambda_type = Type::getFunc(param_type, def_node->type);
                def_node = make_shared<Lambda>(args[i], def_node, lambda_type);
            }
            
            global_nodes_[funcname] = def_node;
        }else{
            throw make_error("Unexpected token in statement");
        }
    }

    // 型パーサー
    TypePtr parse_type(){
        if(isEnd()){ throw make_error("Unexpected end of input while parsing type"); }
        vector<TypePtr> types;
        while(true){
            if(current_token().type == TokenType::IDENTIFIER){
                string name = current_token().identifier_name; consume();
                BaseType bt = Type::str_to_basetype(name);
                if(bt == BaseType::INT){
                    types.push_back(Type::getInt());
                }else if(bt == BaseType::BOOL){
                    types.push_back(Type::getBool());
                }else{
                    throw make_error("Unknown base type: " + name);
                }
            }else if(current_token().type == TokenType::LPAREN){
                consume();
                TypePtr tp = parse_type();
                expect(TokenType::RPAREN);
                types.push_back(tp);
            }else{
                throw make_error("Expected type name or '(' while parsing type");
            }

            if(current_token().type == TokenType::ARROW){
                consume();
            }else{
                break;
            }
        }
        if(types.size() >= 2 || types.back()->base == BaseType::FUNCTION){
            TypePtr ret = types.back(); types.pop_back();
            return Type::createCurriedFunc(types, ret);
        }else{
            return types[0];
        }
    }

    // 型駆動の式パーサー
    // expected_type: この式に期待される型。この型に一致するまで引数を貪欲に消費する。
    NodePtr parse_expr(TypePtr expected_type, map<string, TypePtr>& local_scope){
        NodePtr node;
        
        // 1. 基本的な要素（リテラル、識別子など）のパース
        if(current_token().type == TokenType::NUMBER){
            node = make_shared<IntLit>(current_token().value);
            consume();
        } else if(current_token().type == TokenType::TRUE){
            node = make_shared<BoolLit>(true);
            consume();
        } else if(current_token().type == TokenType::FALSE){
            node = make_shared<BoolLit>(false);
            consume();
        } else if(current_token().type == TokenType::IDENTIFIER){
            string name = current_token().identifier_name;
            TypePtr type = nullptr;
            if(local_scope.count(name)) type = local_scope[name];
            else if(global_types_.count(name)) type = global_types_[name];
            else throw make_error("Undefined variable: " + name);
            
            node = make_shared<Identifier>(name, type);
            consume();
        } else {
            throw make_error("Unexpected token in expression: " + current_token().to_string());
        }

        // 2. 型駆動の結合 (最短一致)
        // 現在のノードが関数型で、かつ期待される型と一致していない場合、引数を消費し続ける
        while(node->type->base == BaseType::FUNCTION){
            // 期待される型と完全に一致したら停止 (最短一致)
            if(expected_type && node->type->equals(expected_type)){
                break;
            }
            
            // 関数型なので引数を適用する
            auto func_type = static_pointer_cast<FuncType>(node->type);
            
            // 引数のパース (引数の型を期待値として再帰呼び出し)
            NodePtr arg = parse_expr(func_type->param, local_scope);
            
            // Applicationノードの構築
            node = make_shared<Application>(node, arg, func_type->ret);
        }
        
        // 最終的な型チェック
        if(expected_type && !node->type->equals(expected_type)){
             throw make_error(
                "Type mismatch: expected " + expected_type->to_string() +
                ", but got " + node->type->to_string()
            );
        }

        return node;
    }

    map<string, TypePtr> global_types_; // 型環境 (private)
    map<string, NodePtr> global_nodes_; // パース結果 (private)

public:
    Parser(const vector<Token>& list) : tokens(list), index(0){}

    // 組み込み型の登録インターフェース
    void register_builtin(const string& name, TypePtr type){
        global_types_[name] = type;
    }

    // パース結果の取得
    const map<string, NodePtr>& get_nodes() const { return global_nodes_; }

    // パース時に型を参照する必要がある場合のアクセサ（読み取り専用）
    const map<string, TypePtr>& get_types() const { return global_types_; }

    void parse_program(){
        while(!isEnd()){
            parse_statement();
        }
    }
};

#endif