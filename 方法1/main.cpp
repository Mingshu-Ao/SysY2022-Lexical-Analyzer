#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::pair;
using std::make_pair;

class Lexer {
private:
    string input;
    int pos;
    char c;
    string token;
    int state;
    int numRow;
    int numChar;
    enum TokenType {
        ID,
        KEYWORD,
        INT_D,
        INT_O,
        INT_H,
        FLOAT_D,
        FLOAT_H,
        RELOP,
        ARITHOP,
        PUNCTUATION,
        ASSIGNOP,
        LOGIC,
        DELIMITER,
        BRACKET,
        COMMENT,
        ERROR,
        END
    };
    map<TokenType, int> numWords; // 记录每个类型对应的数量
    map<string, TokenType> keywords;

    void get_char() {
        if (pos < input.length()) {
            c = input[pos++];
        }
        else {
            c = '\0';
        }
    }


    void get_nbc() {
        if (isspace(c) || c == '\n') {
            if (c == '\n')numRow--;
            while (isspace(c) || c == '\n') {
                if (c == '\n')numRow++; // 会多跑一次
                get_char();
            }
        }
        else pos++;
    }


    void cat() {
        token += c;
    }

    void retract() {
        if (pos > 0) {
            pos--;
        }
    }

    // 检查是否为关键字
    int reserve() {
        if (keywords.find(token) != keywords.end()) {
            return keywords[token];
        }
        return -1;
    }
    bool is_nondigit() {
        return isalpha(c) || c == '_';
    }
    bool is_digit() {
        return isdigit(c);
    }
    bool is_nonzerodigit() {
        return isdigit(c) && c != '0';
    }
    bool is_hexdigit() {
        return isdigit(c) || (c - 'a' >= 0 && c - 'a' <= 5) || (c - 'A' >= 0 && c - 'A' <= 5);
    }
    bool is_octaldigit() {
        return isdigit(c) && c != '8' && c != '9';
    }



    // 错误输出
    void error(const string& msg) {
        cerr << "第" << numRow << "行存在词法错误: " << msg << endl;
    }

    void recordWords(TokenType t) {
        if (numWords.find(t) == numWords.end()) {
            numWords[t] = 1;
        }
        else {
            numWords[t]++;
        }
    }

public:
    Lexer(const string& str) : input(str), pos(0), c(' '), state(0), numRow(1), numChar(0) {
        keywords = {
           {"int", KEYWORD},{"float", KEYWORD},{"void", KEYWORD},{"if", KEYWORD},{"else", KEYWORD},
            {"while", KEYWORD},{"break", KEYWORD},{"continue", KEYWORD},{"return", KEYWORD},{"const", KEYWORD},{"for",KEYWORD},{"char",KEYWORD}
        };
    }

    pair<TokenType, string> get_token() {
        token = "";
        state = 0;

        while (true) {
            switch (state) {
            case 0: // 初始状态
                get_nbc();

                if (is_nondigit()) {
                    cat();
                    get_char();
                    state = 1; // 标识符状态
                }
                else if (is_nonzerodigit()) {
                    cat();
                    get_char();
                    state = 2; // 十进制整数状态
                }
                else if (c == '0') {
                    cat();
                    get_char();
                    if (c == 'x' || c == 'X') {
                        cat();
                        get_char();
                        state = 10;// 十六进制整数状态
                    }
                    else if (is_digit()) {
                        ///
                        state = 7;// 八进制整数状态
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(INT_D);
                        return make_pair(INT_D, token);// 单一为0的情况
                    }

                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 9;// 十进制小数状态
                }




                else if (c == '<' || c == '>') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                }
                else if (c == '=') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                }
                else if (c == '!') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                }
                else if (c == '&') {
                    cat();
                    get_char();
                    if (c == '&') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                    else {// 视为有两个&
                        token += '&';
                        retract();
                        error("单一'&'，不存在此符号");
                        //return make_pair(ERROR, "");
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                }
                else if (c == '|') {
                    cat();
                    get_char();
                    if (c == '|') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                    else {// 视为有两个&
                        token += '|';
                        retract();
                        error("单一'|'，不存在此符号");
                        //return make_pair(ERROR, "");
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                }
                /*else if (c == '<' || c == '>') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                }*/
                else if ( c == '*' || c == '%') { /////修改
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                }
                else if (c == '+' ) {  //////修改
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                    else if (c == '+') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                }
                else if (c == '-') {  //////修改
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                    else if (c == '-') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                }
                else if (c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                    cat();
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(BRACKET);
                    return make_pair(BRACKET, token);
                }
                else if (c == ',' || c == ';') {
                    cat();
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(DELIMITER);
                    return make_pair(DELIMITER, token);
                }



                /*else if (c == '+' ) {  //////修改
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                    else if (c == '+') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);
                    }
                }*/
                else if (c == '/') {
                    cat();
                    get_char();
                    if (c == '/') {
                        cat();
                        get_char();
                        state = 17;
                    }
                    else if (c == '*') {
                        cat();
                        get_char();
                        state = 18;
                    }
                    else if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);// 除号
                    }
                }


                else if (c == '\0') {
                    return make_pair(END, "");
                }
                else {
                    cat();
                    error("未定义的字符");
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);//error定义为未修复错误
                    return make_pair(ERROR, token);
                }
                break;

            case 1: // 标识符状态
                if (is_nondigit() || is_digit()) {
                    cat();
                    get_char();
                }
                else {
                    retract();
                    int keywordCheck = reserve();
                    if (keywordCheck != -1) {
                        numChar += token.size();
                        //return make_pair(static_cast<TokenType>(keywordCheck), token);// 通过枚举从keywordCheck的序号得到对应关键字
                        recordWords(KEYWORD);
                        return make_pair(KEYWORD, token);
                    }
                    else {
                        numChar += token.size();
                        recordWords(ID);
                        return make_pair(ID, token);
                    }
                }
                break;

            case 2: // 十进制整数状态
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 3; // 小数部分状态
                }
                else if (c == 'E' || c == 'e') {
                    cat();
                    get_char();
                    state = 4; // 指数部分状态
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(INT_D);
                    return make_pair(INT_D, token);
                }
                break;

            case 3: // 小数部分状态
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else if (c == 'E' || c == 'e') {
                    cat();
                    get_char();
                    state = 4; // 指数部分状态
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(FLOAT_D);
                    return make_pair(FLOAT_D, token);
                }
                break;

            case 4: // 指数部分状态
                if (c == '+' || c == '-') {
                    cat();
                    get_char();
                    state = 5;
                }
                else if (is_digit()) {
                    cat();
                    get_char();
                    state = 6;
                }
                else {
                    token.erase(token.end() - 1);

                    error("十进制浮点数e/E后格式不正确"); // 把e/E删除，然后判断删除后应该归到哪个类型
                    if (token[0] == '0') {//8->4
                        error("有前导零的十进制整数");
                        retract();
                        token.erase(token.begin());
                        numChar += token.size();
                        recordWords(INT_D);
                        return make_pair(INT_D, token);
                    }
                    else {
                        bool isDot = false;
                        for (auto i : token) {
                            if (i == '.') {
                                isDot = true;
                                break;
                            }
                        }
                        if (isDot) {//3->4
                            retract();
                            numChar += token.size();
                            recordWords(FLOAT_D);
                            return make_pair(FLOAT_D, token);
                        }
                        else {//2->4
                            retract();
                            numChar += token.size();
                            recordWords(INT_D);
                            return make_pair(INT_D, token);
                        }
                    }
                }
                break;
            case 5:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 6;
                }
                else {
                    error("十进制浮点数+/-后格式不正确"); //+-和e删除，输出为十进制整数   或者
                    token.erase(token.end() - 1);//删除+/-和e/E
                    token.erase(token.end() - 1);  // 测试数据：012.1e+*12.1e-*12e+*1
                    if (token[0] == '0') {//8->4
                        error("有前导零的十进制整数");
                        retract();
                        token.erase(token.begin());
                        numChar += token.size();
                        recordWords(INT_D);
                        return make_pair(INT_D, token);
                    }
                    else {
                        bool isDot = false;
                        for (auto i : token) {
                            if (i == '.') {
                                isDot = true;
                                break;
                            }
                        }
                        if (isDot) {//3->4
                            retract();
                            numChar += token.size();
                            recordWords(FLOAT_D);
                            return make_pair(FLOAT_D, token);
                        }
                        else {//2->4
                            retract();
                            numChar += token.size();
                            recordWords(INT_D);
                            return make_pair(INT_D, token);
                        }
                    }
                }
                break;
            case 6: // 小数或指数部分继续状态
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(FLOAT_D);
                    return make_pair(FLOAT_D, token);
                }
                break;
            case 7:
                if (is_octaldigit()) {
                    cat();
                    get_char();
                }
                else if (is_digit()) {
                    cat();
                    get_char();
                    state = 8;
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 3;
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(INT_O);
                    return make_pair(INT_O, token);
                }
                break;
            case 8:
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 3;
                }
                else if (c == 'e' || c == 'E') {
                    cat();
                    get_char();
                    state = 4;
                }
                else {
                    error("有前导零的十进制整数");
                    retract();
                    token.erase(token.begin());
                    numChar += token.size();
                    recordWords(INT_D);
                    return make_pair(INT_D, token);
                }
                break;
            case 9:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 3;
                }
                else {
                    error("十进制浮点数'.'前后不能同时没有数字"); //认为是.0
                    retract();
                    token += '0';
                    numChar += token.size();
                    recordWords(FLOAT_D);
                    return make_pair(FLOAT_D, token);
                }
                break;
            case 10:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                    state = 11;
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 12;
                }
                else {
                    error("十六进制数0x/0X后格式错误");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 11:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 13;
                }
                else if (c == 'p' || c == 'P') {
                    cat();
                    get_char();
                    state = 14;
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(INT_H);
                    return make_pair(INT_H, token);
                }
                break;
            case 12:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                    state = 13;
                }
                else {
                    error("十六进制浮点数'.'后需要十六进制数字");// 认为是.0
                    retract();
                    token += '0';
                    numChar += token.size();
                    recordWords(FLOAT_H);
                    return make_pair(FLOAT_H, token);
                }
                break;
            case 13:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                }
                else if (c == 'p' || c == 'P') {
                    cat();
                    get_char();
                    state = 14;
                }
                else {
                    error("十六进制浮点数'.'后格式错误"); //因为p/P是必须有的，故无法通过删除p/P来修复错误，所以直接报错
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 14:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 16;
                }
                else if (c == '+' || c == '-') {
                    cat();
                    get_char();
                    state = 15;
                }
                else {
                    error("十六进制浮点数p/P后格式错误");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 15:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 16;
                }
                else {
                    error("十六进制浮点数+/-后需要数字");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 16:
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(FLOAT_H);
                    return make_pair(FLOAT_H, token);
                }
                break;

            case 17: // 注释
                if (c != '\n' && c != '\0') {
                    cat();
                    get_char();
                }
                else {
                    numRow++;
                    c = input[pos];
                    recordWords(COMMENT);
                    numChar += token.size();
                    return make_pair(COMMENT, token.substr(2));
                }
                break;
            case 18:
                if (c != '*' && c != '\0') {
                    if (c == '\n')numRow++;
                    cat();
                    get_char();
                }
                else if (c == '\0') {
                    error("注释未闭合");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                else {//c=='*'
                    cat();
                    get_char();
                    state = 19;
                }
                break;
            case 19:
                if (c == '*') {
                    cat();
                    get_char();
                }
                else if (c == '/') {
                    get_char();
                    retract();
                    token.erase(token.end() - 1);//去掉最后的*
                    numChar += token.size() + 2;
                    recordWords(COMMENT);
                    return make_pair(COMMENT, token.substr(2));
                }
                else if (c == '\0') {
                    error("注释未闭合");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                else {
                    cat();
                    get_char();
                    state = 18;
                }
                break;
            default:
                error("Invalid state");
                return make_pair(END, "");
            }
        }
    }

    void output() {
        pair<TokenType, string> token;
        do {
            token = get_token();
            switch (token.first) {
            case ID: cout  << token.second << "标识符" << endl; break;
            case KEYWORD: cout <<  token.second << "关键字" << endl; break;
            case INT_D: cout <<  token.second << "常数" << endl; break;
            case INT_O: cout <<  token.second << "常数" << endl; break;
            case INT_H: cout << token.second << "常数" << endl; break;
            case FLOAT_D: cout << token.second << "常数" << endl; break;
            case FLOAT_H: cout <<  token.second << "常数" << endl; break;
            case RELOP: cout <<  token.second << "运算符" << endl; break;
            case ARITHOP: cout <<  token.second << "运算符" << endl; break;
            case PUNCTUATION: cout << "<PUNCTUATION, " << token.second << ">" << endl; break;
            case ASSIGNOP: cout <<  token.second << "赋值" << endl; break;
            case LOGIC: cout << "<LOGIC, " << token.second << ">" << endl; break;
            case DELIMITER: cout <<  token.second << "分隔符" << endl; break;
            case BRACKET: cout << token.second << "分隔符" << endl; break;
            case COMMENT:  break;
            case ERROR: cout << "<ERROR, " << token.second << ">" << endl; break;
            case END: break;
            default: cout << "<UNKNOWN, " << token.second << ">" << endl; break;
            }
        } while (token.first != END);
    }
    int get_numRow() {
        return numRow;
    }
    int get_numChar() {
        return numChar;
    }
    int get_totalNumWords() {
        int res=0;
        for (auto [key, val] : numWords) {
            if(key!=PUNCTUATION&&key!=COMMENT&&key!=END&&key!=ERROR)
            res += val;
        }
        return res;
    }
    void printNumWords() {
        int num_key = 0, num_id = 0, num_int = 0, num_assign = 0, num_ari = 0, num_deli = 0;
        for(auto[key,val]:numWords) {

            switch (key) {
            /* ID: cout << val << "标识符" << endl; break;
            case KEYWORD: cout << val << "关键字" << endl; break;
            case INT_D: cout <<  val << "常数" << endl; break;
            case INT_O: cout <<  val << "常数" << endl; break;
            case INT_H: cout <<  val << "常数" << endl; break;
            case FLOAT_D: cout << val << "常数" << endl; break;
            case FLOAT_H: cout <<  val << "常数" << endl; break;
            case RELOP: cout <<  val << "运算符" << endl; break;
            case ARITHOP: cout <<  val << "运算符" << endl; break;
            case PUNCTUATION: cout << "<PUNCTUATION, " << val << ">" << endl; break;
            case ASSIGNOP: cout <<  val << "赋值" << endl; break;
            case LOGIC: cout << val << "赋值" << endl; break;
            case DELIMITER: cout <<  val << "分隔符" << endl; break;
            case BRACKET: cout <<  val << "分隔符" << endl; break;
            case COMMENT:  break;
            case ERROR: cout << "<ERROR, " << val << ">" << endl; break;
            case END: break;
            default: cout << "<UNKNOWN, " << val << ">" << endl; break;*/
            case ID: num_id += val; break;
            case KEYWORD: num_key += val; break;
            case INT_D: num_int += val; break;
            case INT_O: num_int += val; break;
            case INT_H: num_int += val; break;
            case FLOAT_D: num_int += val; break;
            case FLOAT_H: num_int += val; break;
            case RELOP: num_ari += val; break;
            case ARITHOP: num_ari += val; break;
            case PUNCTUATION: cout << "<PUNCTUATION, " << val << ">" << endl; break;
            case ASSIGNOP: num_assign += val; break;
            case LOGIC: num_ari += val; break;
            case DELIMITER: num_deli += val; break;
            case BRACKET: num_deli += val; break;
            case COMMENT:  break;
            case ERROR: cout << "<ERROR, " << val << ">" << endl; break;
            case END: break;
            default: cout << "<UNKNOWN, " << val << ">" << endl; break;
            }
        }
        cout << num_key << "关键字\n" << num_id << "标识符\n" << num_int << "常数\n" << num_assign << "赋值\n" << num_ari << "运算符\n" << num_deli << "分隔符";
    }
};

// 测试函数
int main() {
    string input = "\nint main(){if(i==1)\n\n{break;}else i=2; 1.1 1.e123  ^  0x+1 .+1 0 01. .1E123 123.  \n   \n  012.1e+*12.1e-*12e+*1   012.1e*12.1e*12e*1  0x1 0x123.1p+123 0x.123p1 //*aodjoiawjdoiajw \n 123.   01  /*aedjiowai*djio*awj\nkj*daij 1 */ 1/3}";
    Lexer lexer(input);
    lexer.output();
    cout <<  lexer.get_numRow() <<"行" << endl;
    //cout << "字符总数：" << lexer.get_numChar() << endl;
    cout << lexer.get_totalNumWords()<<"个词语" << endl;

    lexer.printNumWords();
    return 0;
}

//每次输出后pos所指位置元素和c相同，且为token的下一个字符位置
//\nnt main(){if(i==1)\n\n{break;}else i=2; 1.1 1.e123 .1 .1E123 123.  \n   \n      0x1 0x123.1p+123 0x.123p1 //*aodjoiawjdoiajw \n 123.  0 01.  /*aedjiowai*djio*awj\nkj*daij 1 */ 2 / 3}

//可能的统一修复错误逻辑：删除出错部分，接着读入字符，看token能否正确输出  或者直接删除后输出token，再重新看后面的部分


/*
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::pair;
using std::make_pair;

class Lexer {
private:
    string input;     
    int pos;           
    char c;             
    string token;      
    int state;         
    int numRow;
    int numChar;
    enum TokenType {
        ID,         
        KEYWORD,     
        INT_D,
        INT_O,
        INT_H,
        FLOAT_D,
        FLOAT_H,      
        RELOP,       
        ARITHOP,     
        PUNCTUATION, 
        ASSIGNOP,   
        LOGIC,
        DELIMITER,
        BRACKET,
        COMMENT,     
        ERROR,
        END        
    };
    map<TokenType, int> numWords; // 记录每个类型对应的数量
    map<string, TokenType> keywords;

    void get_char() {
        if (pos < input.length()) {
            c = input[pos++];
        }
        else {
            c = '\0';
        }
    }


    void get_nbc() {
        if (isspace(c) || c == '\n') {
            if (c == '\n')numRow--;
            while (isspace(c) || c == '\n') {
                if (c == '\n')numRow++; // 会多跑一次
                get_char();
            }
        }
        else pos++;
    }

 
    void cat() {
        token += c;
    }

    void retract() {
        if (pos > 0) {
            pos--;
        }
    }

    // 检查是否为关键字
    int reserve() {
        if (keywords.find(token) != keywords.end()) {
            return keywords[token];
        }
        return -1;
    }
    bool is_nondigit() {
        return isalpha(c) || c == '_';
    }
    bool is_digit() {
        return isdigit(c);
    }
    bool is_nonzerodigit() {
        return isdigit(c) && c != '0';
    }
    bool is_hexdigit() {
        return isdigit(c) || (c - 'a' >= 0 && c - 'a' <= 5) || (c - 'A' >= 0 && c - 'A' <= 5);
    }
    bool is_octaldigit() {
        return isdigit(c) && c != '8' && c != '9';
    }
    


    // 错误输出
    void error(const string& msg) {
        cerr << "第"<< numRow << "行存在词法错误: " << msg << endl;
    }

    void recordWords(TokenType t) {
        if (numWords.find(t) == numWords.end()) {
            numWords[t] = 1;
        }
        else {
            numWords[t]++;
        }
    }

public:
    Lexer(const string& str) : input(str), pos(0), c(' '), state(0) ,numRow(1),numChar(0){
        keywords= {
            {"main", KEYWORD},{"int", KEYWORD},{"float", KEYWORD},{"void", KEYWORD},{"if", KEYWORD},{"else", KEYWORD},
            {"while", KEYWORD},{"break", KEYWORD},{"continue", KEYWORD},{"return", KEYWORD},{"const", KEYWORD}
        };
    }

    pair<TokenType, string> get_token() {
        token = "";
        state = 0;

        while (true) {
            switch (state) {
            case 0: // 初始状态
                get_nbc();

                if (is_nondigit()) {
                    cat();
                    get_char();
                    state = 1; // 标识符状态
                }
                else if (is_nonzerodigit()) {
                    cat();
                    get_char();
                    state = 2; // 十进制整数状态
                }
                else if (c == '0') {
                    cat();
                    get_char();
                    if (c == 'x' || c == 'X') {
                        cat();
                        get_char();
                        state = 10;// 十六进制整数状态
                    }
                    else if(is_digit()){
                        ///
                        state = 7;// 八进制整数状态
                    }
                    else {
                        retract();
                        numChar += token.size();
                        return make_pair(INT_D, token);// 单一为0的情况
                    }
                
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 9;// 十进制小数状态
                }




                else if (c == '<' || c == '>') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                }
                else if (c == '=') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ASSIGNOP);
                        return make_pair(ASSIGNOP, token);
                    }
                }
                else if (c == '!') {
                    cat();
                    get_char();
                    if (c == '=') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(RELOP);
                        return make_pair(RELOP, token);
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                }
                else if (c == '&') {
                    cat();
                    get_char();
                    if (c == '&') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                    else {// 视为有两个&
                        token += '&';
                        retract();
                        error("单一'&'，不存在此符号");
                        //return make_pair(ERROR, "");
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                }
                else if (c == '|') {
                    cat();
                    get_char();
                    if (c == '|') {
                        cat();
                        get_char();
                        retract();
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                    else {// 视为有两个&
                        token += '|';
                        retract();
                        error("单一'|'，不存在此符号");
                        //return make_pair(ERROR, "");
                        numChar += token.size();
                        recordWords(LOGIC);
                        return make_pair(LOGIC, token);
                    }
                }
                else if (c == '+' || c == '-' || c == '*' ||c=='%') {
                    cat();
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(ARITHOP);
                    return make_pair(ARITHOP, token);
                }
                else if ( c == '(' || c == ')' ||c=='['||c==']'||c=='{'||c=='}') {
                    cat();
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(BRACKET);
                    return make_pair(BRACKET, token);
                }
                else if (c==',' || c == ';') {
                    cat();
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(DELIMITER);
                    return make_pair(DELIMITER, token);
                }
        



                else if (c == '/') {
                    cat();
                    get_char();
                    if (c == '/') {
                        cat();
                        get_char();
                        state = 17;
                    }
                    else if (c == '*') {
                        cat();
                        get_char();
                        state = 18; 
                    }
                    else {
                        retract();
                        numChar += token.size();
                        recordWords(ARITHOP);
                        return make_pair(ARITHOP, token);// 除号
                    }
                }
                
             
                else if (c == '\0') {
                    return make_pair(END, "");
                }
                else {
                    cat();
                    error("未定义的字符");
                    get_char();
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);//error定义为未修复错误
                    return make_pair(ERROR, token);
                }
                break;

            case 1: // 标识符状态
                if (is_nondigit() || is_digit()) {
                    cat();
                    get_char();
                }
                else {
                    retract();
                    int keywordCheck = reserve();
                    if (keywordCheck != -1) {
                        numChar += token.size();
                        //return make_pair(static_cast<TokenType>(keywordCheck), token);// 通过枚举从keywordCheck的序号得到对应关键字
                        recordWords(KEYWORD);
                        return make_pair(KEYWORD, token);
                    }
                    else {
                        numChar += token.size();
                        recordWords(ID);
                        return make_pair(ID, token);
                    }
                }
                break;

            case 2: // 十进制整数状态
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 3; // 小数部分状态
                }
                else if (c == 'E' || c == 'e') {
                    cat();
                    get_char();
                    state = 4; // 指数部分状态
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(INT_D);
                    return make_pair(INT_D, token);
                }
                break;

            case 3: // 小数部分状态
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else if (c == 'E' || c == 'e') {
                    cat();
                    get_char();
                    state = 4; // 指数部分状态
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(FLOAT_D);
                    return make_pair(FLOAT_D, token);
                }
                break;

            case 4: // 指数部分状态
                if (c == '+' || c == '-') {
                    cat();
                    get_char();
                    state = 5;
                }
                else if (is_digit()) {
                    cat();
                    get_char();
                    state = 6;
                }
                else {
                    token.erase(token.end() - 1);
                    
                    error("十进制浮点数e/E后格式不正确"); // 把e/E删除，然后判断删除后应该归到哪个类型
                    if (token[0] == '0') {//8->4
                        error("有前导零的十进制整数");
                        retract();
                        token.erase(token.begin());
                        numChar += token.size();
                        recordWords(INT_D);
                        return make_pair(INT_D, token);
                    }
                    else {
                        bool isDot = false;
                        for (auto i : token) {
                            if (i == '.') {
                                isDot = true;
                                break;
                            }
                        }
                        if (isDot) {//3->4
                            retract();
                            numChar += token.size();
                            recordWords(FLOAT_D);
                            return make_pair(FLOAT_D, token);
                        }
                        else {//2->4
                            retract();
                            numChar += token.size();
                            recordWords(INT_D);
                            return make_pair(INT_D, token);
                        }
                    }
                }
                break;
            case 5: 
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 6;
                }
                else {
                    error("十进制浮点数+/-后格式不正确"); //+-和e删除，输出为十进制整数   或者
                    token.erase(token.end() - 1);//删除+/-和e/E
                    token.erase(token.end() - 1);  // 测试数据：012.1e+*12.1e-*12e+*1
                    if (token[0] == '0') {//8->4
                        error("有前导零的十进制整数");
                        retract();
                        token.erase(token.begin());
                        numChar += token.size();
                        recordWords(INT_D);
                        return make_pair(INT_D, token);
                    }
                    else {
                        bool isDot = false;
                        for (auto i : token) {
                            if (i == '.') {
                                isDot = true;
                                break;
                            }
                        }
                        if (isDot) {//3->4
                            retract();
                            numChar += token.size();
                            recordWords(FLOAT_D);
                            return make_pair(FLOAT_D, token);
                        }
                        else {//2->4
                            retract();
                            numChar += token.size();
                            recordWords(INT_D);
                            return make_pair(INT_D, token);
                        }
                    }
                }
                break;
            case 6: // 小数或指数部分继续状态
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(FLOAT_D);
                    return make_pair(FLOAT_D, token);
                }
                break;
            case 7:
                if (is_octaldigit()) {
                    cat();
                    get_char();
                }
                else if (is_digit()) {
                    cat();
                    get_char();
                    state = 8;
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 3;
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(INT_O);
                    return make_pair(INT_O, token);
                }
                break;
            case 8:
                if (is_digit()){
                    cat();
                    get_char();
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 3;
                }
                else if (c == 'e' || c == 'E') {
                    cat();
                    get_char();
                    state = 4;
                }
                else {
                    error("有前导零的十进制整数");
                    retract();
                    token.erase(token.begin());
                    numChar += token.size();
                    recordWords(INT_D);
                    return make_pair(INT_D, token);
                }
                break;
            case 9:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 3;
                }
                else {
                    error("十进制浮点数'.'前后不能同时没有数字"); //认为是.0
                    retract();
                    token += '0';
                    numChar += token.size();
                    recordWords(FLOAT_D);
                    return make_pair(FLOAT_D, token);
                }
                break;
            case 10:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                    state = 11;
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 12;
                }
                else {
                    error("十六进制数0x/0X后格式错误");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 11:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                }
                else if (c == '.') {
                    cat();
                    get_char();
                    state = 13;
                }
                else if (c == 'p' || c == 'P') {
                    cat();
                    get_char();
                    state = 14;
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(INT_H);
                    return make_pair(INT_H, token);
                }
                break;
            case 12:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                    state = 13;
                }
                else {
                    error("十六进制浮点数'.'后需要十六进制数字");// 认为是.0
                    retract();
                    token += '0';
                    numChar += token.size();
                    recordWords(FLOAT_H);
                    return make_pair(FLOAT_H, token);
                }
                break;
            case 13:
                if (is_hexdigit()) {
                    cat();
                    get_char();
                }
                else if (c == 'p' || c == 'P') {
                    cat();
                    get_char();
                    state = 14;
                }
                else {
                    error("十六进制浮点数'.'后格式错误"); //因为p/P是必须有的，故无法通过删除p/P来修复错误，所以直接报错
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 14:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 16;
                }
                else if (c == '+' || c == '-') {
                    cat();
                    get_char();
                    state = 15;
                }
                else {
                    error("十六进制浮点数p/P后格式错误");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 15:
                if (is_digit()) {
                    cat();
                    get_char();
                    state = 16;
                }               
                else {
                    error("十六进制浮点数+/-后需要数字");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                break;
            case 16:
                if (is_digit()) {
                    cat();
                    get_char();
                }
                else {
                    retract();
                    numChar += token.size();
                    recordWords(FLOAT_H);
                    return make_pair(FLOAT_H, token);
                }
                break;
                
            case 17: // 注释
                if (c != '\n'&&c!='\0') {
                    cat();
                    get_char();
                }
                else {
                    numRow++;
                    c = input[pos];
                    recordWords(COMMENT);
                    numChar += token.size();
                    return make_pair(COMMENT, token.substr(2));
                }
                break;
            case 18:
                if (c != '*'&&c!='\0') {
                    if (c == '\n')numRow++;
                    cat();
                    get_char();
                }
                else if (c == '\0') {
                    error("注释未闭合");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                else {//c=='*'
                    cat();
                    get_char();
                    state = 19;
                }
                break;
            case 19:
                if (c == '*') {
                    cat();
                    get_char();
                }
                else if (c == '/') {
                    get_char();
                    retract();
                    token.erase(token.end() - 1);//去掉最后的*
                    numChar += token.size() + 2;
                    recordWords(COMMENT);
                    return make_pair(COMMENT, token.substr(2));
                }
                else if (c == '\0') {
                    error("注释未闭合");
                    retract();
                    numChar += token.size();
                    recordWords(ERROR);
                    return make_pair(ERROR, token);
                }
                else {
                    cat();
                    get_char();
                    state = 18;
                }
                break;           
            default:
                error("Invalid state");
                return make_pair(END, "");
            }
        }
    }

    void output() {
        pair<TokenType, string> token;
        do {
            token = get_token();
            switch (token.first) {
                case ID: cout << "<ID, " << token.second << ">" << endl; break;
                case KEYWORD: cout << "<KEYWORD, " << token.second << ">" << endl; break;
                case INT_D: cout << "<INT_D, " << token.second << ">" << endl; break;
                case INT_O: cout << "<INT_O, " << token.second << ">" << endl; break;
                case INT_H: cout << "<INT_H, " << token.second << ">" << endl; break;
                case FLOAT_D: cout << "<FLOAT_D, " << token.second << ">" << endl; break;
                case FLOAT_H: cout << "<FLOAT_H, " << token.second << ">" << endl; break;
                case RELOP: cout << "<RELOP, " << token.second << ">" << endl; break;
                case ARITHOP: cout << "<ARITHOP, " << token.second << ">" << endl; break;
                case PUNCTUATION: cout << "<PUNCTUATION, " << token.second << ">" << endl; break;
                case ASSIGNOP: cout << "<ASSIGNOP, " << token.second << ">" << endl; break;
                case LOGIC: cout << "<LOGIC, " << token.second << ">" << endl; break;
                case DELIMITER: cout << "<DELIMETER, " << token.second << ">" << endl; break;
                case BRACKET: cout << "<BRACKET, " << token.second << ">" << endl; break;
                case COMMENT: cout << "<COMMENT, " << token.second << ">" << endl; break;
                case ERROR: cout << "<ERROR, " << token.second << ">" << endl; break;
                case END: cout << "<END, " << token.second << ">" << endl; break;
                default: cout << "<UNKNOWN, " << token.second << ">" << endl; break;
            }
        } while (token.first != END);
    }
    int get_numRow() {
        return numRow;
    }
    int get_numChar() {
        return numChar;
    }
    void printNumWords() {
        for (auto [key, val] : numWords) {
            switch (key) {
            case ID: cout << "<ID, " << val<< ">" << endl; break;
            case KEYWORD: cout << "<KEYWORD, " << val << ">" << endl; break;
            case INT_D: cout << "<INT_D, " << val << ">" << endl; break;
            case INT_O: cout << "<INT_O, " << val << ">" << endl; break;
            case INT_H: cout << "<INT_H, " << val << ">" << endl; break;
            case FLOAT_D: cout << "<FLOAT_D, " << val << ">" << endl; break;
            case FLOAT_H: cout << "<FLOAT_H, " << val << ">" << endl; break;
            case RELOP: cout << "<RELOP, " << val << ">" << endl; break;
            case ARITHOP: cout << "<ARITHOP, " << val << ">" << endl; break;
            case PUNCTUATION: cout << "<PUNCTUATION, " << val << ">" << endl; break;
            case ASSIGNOP: cout << "<ASSIGNOP, " << val << ">" << endl; break;
            case LOGIC: cout << "<LOGIC, " << val << ">" << endl; break;
            case DELIMITER: cout << "<DELIMETER, " << val << ">" << endl; break;
            case BRACKET: cout << "<BRACKET, " << val << ">" << endl; break;
            case COMMENT: cout << "<COMMENT, " << val << ">" << endl; break;
            case ERROR: cout << "<ERROR, " << val<< ">" << endl; break;
            case END: cout << "<END, " << val << ">" << endl; break;
            default: cout << "<UNKNOWN, " << val << ">" << endl; break;
            }
        }
    }
};

// 测试函数
int main() {
    string input = "\nint main(){if(i==1)\n\n{break;}else i=2; 1.1 1.e123  ^  0x+1 .+1 0 01. .1E123 123.  \n   \n  012.1e+*12.1e-*12e+*1   012.1e*12.1e*12e*1  0x1 0x123.1p+123 0x.123p1 //*aodjoiawjdoiajw \n 123.   01  /*aedjiowai*djio*awj\nkj*daij 1 */ //1/3}";
    /*Lexer lexer(input);
    lexer.output();
    cout <<"语句行数：" << lexer.get_numRow() << endl;
    cout << "字符总数：" << lexer.get_numChar() << endl;
    cout << "各类单词的数目：" << endl;
    lexer.printNumWords();
    return 0;
}*/

//每次输出后pos所指位置元素和c相同，且为token的下一个字符位置
//\nnt main(){if(i==1)\n\n{break;}else i=2; 1.1 1.e123 .1 .1E123 123.  \n   \n      0x1 0x123.1p+123 0x.123p1 //*aodjoiawjdoiajw \n 123.  0 01.  /*aedjiowai*djio*awj\nkj*daij 1 */ 2 / 3}

//可能的统一修复错误逻辑：删除出错部分，接着读入字符，看token能否正确输出  或者直接删除后输出token，再重新看后面的部分
