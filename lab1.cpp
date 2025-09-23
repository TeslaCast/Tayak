#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <cctype>
#include <cmath>
#include <stdexcept>

using namespace std;

// ????????? ??? ????????
int getPriority(const string& op) {
    if (op == "+" || op == "-") return 1;   
    if (op == "*" || op == "/") return 2;   
    if (op == "^") return 3;                
    if (op == "pow") return 4;              
    return 0;
}

// ???????? ?? ????????
bool isOperator(const string& t) {
    return t == "+" || t == "-" || t == "*" || t == "/" || t == "^";
}

// ????????? ?? ????????????
vector<string> tokenize(const string& expr) {
    vector<string> tokens;          
    for (size_t i = 0; i < expr.size();) {
        unsigned char ch = expr[i]; 

        // - ???????
        if (isspace(ch)) { i++; continue; }

        // ????????? + ? - (????? ???? ????????)
        if (ch == '+' || ch == '-') {
            bool unary = tokens.empty() || isOperator(tokens.back()) || tokens.back() == "(" || tokens.back() == ",";
            if (unary) {
                // ????? ? ??????, ????. -2
                if (i + 1 < expr.size() && (isdigit((unsigned char)expr[i+1]) || expr[i+1] == '.')) {
                    string num(1, ch); // ?????????? ????
                    i++;
                    while (i < expr.size() && (isdigit((unsigned char)expr[i]) || expr[i] == '.'))
                        num.push_back(expr[i++]); // ?????? ?????
                    tokens.push_back(num);
                    continue;
                } else {
                    // ??????? ????? ????? ??????? ??? ???????? ? ?????????? ? 0 - (...)
                    tokens.push_back("0");
                    tokens.push_back(string(1, ch));
                    i++;
                    continue;
                }
            }
            // ??????? ???????? + ??? -
            tokens.push_back(string(1, ch));
            i++;
            continue;
        }

        // ????? (??? ?????)
        if (isdigit(ch) || ch == '.') {
            string num;
            while (i < expr.size() && (isdigit((unsigned char)expr[i]) || expr[i] == '.'))
                num.push_back(expr[i++]);
            tokens.push_back(num);
            continue;
        }

        // ??????? ??? ??? (pow)
        if (isalpha(ch)) {
            string id;
            while (i < expr.size() && isalpha((unsigned char)expr[i]))
                id.push_back(expr[i++]);
            tokens.push_back(id);
            continue;
        }

        // ??? ????????? ????????? ??????? (??????, ???????, ?????????)
        tokens.push_back(string(1, ch));
        i++;
    }
    return tokens;
}

// --- ??????? ? ???????? ???????? ?????? (RPN) ---
vector<string> toRPN(const vector<string>& tokens) {
    vector<string> out;      // ???????? ???????
    stack<string> ops;       // ???? ??????????

    for (auto& token : tokens) {
        // ???? ?????
        if (isdigit((unsigned char)token[0]) || token[0] == '.' || 
           ((token[0] == '+' || token[0] == '-') && token.size() > 1))
        {
            out.push_back(token);
        }
        // ???????
        else if (isalpha((unsigned char)token[0])) {
            ops.push(token);
        }
        // ??????? (??????????? ?????????? ???????)
        else if (token == ",") {
            while (!ops.empty() && ops.top() != "(") {
                out.push_back(ops.top()); ops.pop();
            }
        }
        // ????????
        else if (isOperator(token)) {
            while (!ops.empty() && isOperator(ops.top())) {
                int topPr = getPriority(ops.top());
                int curPr = getPriority(token);
                // ^ — ??????????????????
                if (topPr > curPr || (topPr == curPr && token != "^")) {
                    out.push_back(ops.top()); ops.pop();
                } else break;
            }
            ops.push(token);
        }
        // ??????????? ??????
        else if (token == "(") {
            ops.push(token);
        }
        // ??????????? ??????
        else if (token == ")") {
            while (!ops.empty() && ops.top() != "(") {
                out.push_back(ops.top()); ops.pop();
            }
            if (ops.empty()) throw runtime_error("miss ')'");
            ops.pop(); // ?????? "("
            // ???? ????? ??????? ???? ??????? ? ???????? ??
            if (!ops.empty() && isalpha((unsigned char)ops.top()[0])) {
                out.push_back(ops.top()); ops.pop();
            }
        }
        else throw runtime_error("Unknown token: " + token);
    }

    // ?????????? ??? ?? ?????
    while (!ops.empty()) {
        if (ops.top() == "(") throw runtime_error("Mismatched parentheses");
        out.push_back(ops.top()); ops.pop();
    }
    return out;
}

// --- ?????????? ????????? ? RPN ---
double evalRPN(const vector<string>& rpn) {
    stack<double> st;  // ???? ?????

    for (auto& token : rpn) {
        // ?????
        if (isdigit((unsigned char)token[0]) || token[0] == '.' || 
           ((token[0] == '+' || token[0] == '-') && token.size() > 1))
        {
            st.push(stod(token));
        }
        // ?????????
        else if (isOperator(token)) {
            if (st.size() < 2) throw runtime_error("Not enough operands");
            double b = st.top(); st.pop();
            double a = st.top(); st.pop();
            if (token == "+") st.push(a + b);
            else if (token == "-") st.push(a - b);
            else if (token == "*") st.push(a * b);
            else if (token == "/") {
                if (b == 0) throw runtime_error("Division by zero");
                st.push(a / b);
            }
            else if (token == "^") st.push(pow(a, b));
        }
        // ??????? pow
        else if (token == "pow") {
            if (st.size() < 2) throw runtime_error("pow requires two args");
            double b = st.top(); st.pop();
            double a = st.top(); st.pop();
            st.push(pow(a, b));
        }
        else throw runtime_error("Unknown token: " + token);
    }
    if (st.size() != 1) throw runtime_error("Evaluation error");
    return st.top();
}

// --- ????????? ?????? ????????? ---
void process(const string& expr) {
    try {
        auto tokens = tokenize(expr);   // ???????????
        auto rpn = toRPN(tokens);       // ? ???
        cout << "Expr: " << expr << "\nRPN: ";
        for (auto& t : rpn) cout << t << " ";
        cout << "\n";
        cout << "Result = " << evalRPN(rpn) << "\n";
    } catch (exception& e) {
        cout << "Error: " << e.what() << "\n";
    }
    cout << "-------------------\n";
}

// --- ????? ---
void runTests() {
    vector<string> tests = {
        "-1+2", "1+-2", "2*-3", "-(1+2)",
        "pow(-2,3)", "pow(2,-3)", "1+2*(3-4)",
        "(12.5+7)/3", "((1+2)", "1/0",
        "3.14.15+2", "5+", "2^3^2", "10+abc"
    };
    for (auto& e : tests) process(e);
}

// --- main ---
int main() {
    cout << "1 - Enter expression\n2 - Run tests\nChoice: ";
    int c; cin >> c; cin.ignore();
    if (c == 1) { string e; getline(cin,e); process(e); }
    else if (c == 2) runTests();
    else cout << "Invalid choice\n";
}
