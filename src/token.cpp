#ifndef _TOKEN_
#define _TOKEN_

#include "include_all"
using namespace std;

enum class TokenType {
    IDENTIFIER,   // 識別子 (変数名, 関数名, 演算子)
    NUMBER,       // 整数リテラル
    LPAREN,       // (
    RPAREN,       // )
    TRUE,         // true
    FALSE,        // false
    ARROW,        // -> 型シグネチャ用
    DOUBLE_COLON, // :: 型シグネチャ用
    EQUAL,        // = 関数定義用
    SEMICOLON,    // ; 文の終わり
    END           // ファイル終端
};


// トークンの定義
struct Token {
public:
    Token(TokenType t, int v, string id, int ln, int cl): type(t), value(v), identifier_name(id), line(ln), column(cl){

    }
    TokenType type;         // トークンの種類
    int value = 0;          // NUMBERの場合の数値
    string identifier_name; // IDENTIFIERの場合の名前 (e.g. "add", "+")
    int line = -1;          // デバッグ用に位置情報を記録
    int column = -1;
    string to_string() const{
        stringstream ss;
        ss << "type:" << type_to_string(type) << " value:" << value << " identifier_name:" << identifier_name << " line:" << line << " column:" << column;
        return  ss.str();
    }
    const string& type_to_string(TokenType t) const{
        return (map_type_strings.at(t));
    }
private:
    inline static const map<TokenType, string> map_type_strings = {
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::NUMBER, "NUMBER"},
        {TokenType::LPAREN, "LPAREN"},
        {TokenType::RPAREN, "RPAREN"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::ARROW, "ARROW"},
        {TokenType::DOUBLE_COLON, "DOUBLE_COLON"},
        {TokenType::EQUAL, "EQUAL"},
        {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::END, "END"}
    };
};

#endif