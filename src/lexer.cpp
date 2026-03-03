#ifndef _LEXER_
#define _LEXER_

#include "include_all"
using namespace std;

#include "token.cpp"

class Lexer{
public:
    vector<Token> tokenize(const string& input){
        vector<Token> tokens;
        int line = 1;
        int column = 1;
        int pos = 0;
        const int length = input.length();

        while(pos < length){
            char current = input[pos];
            // 空白スキップ
            if(isspace(current)){
                if(current == '\n'){
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                pos++;
                continue;
            }

            // 数字 (NUMBER)
            if(isdigit(current)){
                string num_str;
                int start_col = column;
                while(pos < length && isdigit(input[pos])){
                    num_str += input[pos];
                    pos++;
                    column++;
                }
                tokens.push_back(Token(TokenType::NUMBER, stoi(num_str), "", line, start_col));
                continue;
            }

            // 識別子 (IDENTIFIER) アルファベットまたはアンダースコアで始まる
            if(isalpha(current) || current == '_'){
                string id_str;
                int start_col = column;
                while(pos < length && (isalnum(input[pos]) || input[pos] == '_')){
                    id_str += input[pos];
                    pos++;
                    column++;
                }
                if (id_str == "true") tokens.push_back(Token(TokenType::TRUE, 0, "true", line, start_col));
                else if (id_str == "false") tokens.push_back(Token(TokenType::FALSE, 0, "false", line, start_col));
                else tokens.push_back(Token(TokenType::IDENTIFIER, 0, id_str, line, start_col));
                continue;
            }

            // 記号・演算子
            int start_col = column;
            if(current == '(') { tokens.push_back(Token(TokenType::LPAREN, 0, "(", line, start_col)); pos++; column++; }
            else if(current == ')') { tokens.push_back(Token(TokenType::RPAREN, 0, ")", line, start_col)); pos++; column++; }
            else if(current == ';') { tokens.push_back(Token(TokenType::SEMICOLON, 0, ";", line, start_col)); pos++; column++; }
            else if(current == '=' && pos + 1 < length && input[pos+1] == '=') { 
                tokens.push_back(Token(TokenType::IDENTIFIER, 0, "==", line, start_col)); pos+=2; column+=2; 
            }
            else if(current == '=') { tokens.push_back(Token(TokenType::EQUAL, 0, "=", line, start_col)); pos++; column++; }
            else if(current == ':' && pos + 1 < length && input[pos+1] == ':') { 
                tokens.push_back(Token(TokenType::DOUBLE_COLON, 0, "::", line, start_col)); pos+=2; column+=2; 
            }
            else if(current == '-' && pos + 1 < length && input[pos+1] == '>') {
                tokens.push_back(Token(TokenType::ARROW, 0, "->", line, start_col)); pos+=2; column+=2; 
            }
            // 2文字比較演算子 (>=, <=, !=) は単一文字より先にチェック
            else if(current == '>' && pos + 1 < length && input[pos+1] == '=') {
                tokens.push_back(Token(TokenType::IDENTIFIER, 0, ">=", line, start_col)); pos+=2; column+=2;
            }
            else if(current == '<' && pos + 1 < length && input[pos+1] == '=') {
                tokens.push_back(Token(TokenType::IDENTIFIER, 0, "<=", line, start_col)); pos+=2; column+=2;
            }
            else if(current == '!' && pos + 1 < length && input[pos+1] == '=') {
                tokens.push_back(Token(TokenType::IDENTIFIER, 0, "!=", line, start_col)); pos+=2; column+=2;
            }
            // 演算子 (+, -, *, /, %, <, >) は IDENTIFIER として扱う
            else if(string("+-*/%<>").find(current) != string::npos) {
                tokens.push_back(Token(TokenType::IDENTIFIER, 0, string(1, current), line, start_col)); pos++; column++;
            }
            else {
                cerr << "Unknown character: " << current << " at " << line << ":" << column << endl;
                pos++; column++;
            }
        }
        tokens.push_back(Token(TokenType::END, 0, "", line, column));
        return tokens;
    }
};

#endif