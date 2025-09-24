#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <cctype>
#include <cmath>
#include <stdexcept>

using namespace std;

// getting priority of operation
int getPriority(const string& op) {
    if (op == "+" || op == "-") return 1;   
    if (op == "*" || op == "/") return 2;   
    if (op == "^") return 3;                
    if (op == "pow") return 4;              
    return 0;
}

// Check - atom is operator
bool isOperator(const string& t) {
    return t == "+" || t == "-" || t == "*" || t == "/" || t == "^";
}

// Split input string into atoms
vector<string> atomize(const string& expr) {
    vector<string> atoms;          
    for (size_t i = 0; i < expr.size();) {
        unsigned char ch = expr[i]; 

        // skip spaces
        if (isspace(ch)) { i++; continue; }

        // Numbers int or float
        if (isdigit(ch) || ch == '.') {
            string num;
            while (i < expr.size() && (isdigit((unsigned char)expr[i]) || expr[i] == '.'))
                num.push_back(expr[i++]);
            atoms.push_back(num);
            continue;
        }

        // collect function fr o
        if (isalpha(ch)) {
            string id;
            while (i < expr.size() && isalpha((unsigned char)expr[i]))
                id.push_back(expr[i++]);
            atoms.push_back(id);
            continue;
        }


        atoms.push_back(string(1, ch));
        i++;
    }
    return atoms;
}

// Convert atoms to rpn
vector<string> toRPN(const vector<string>& atoms) {
    vector<string> out;      
    stack<string> ops;       

    //add to stack
    for (auto& atom : atoms) {
        // Number
        if (isdigit((unsigned char)atom[0]) || atom[0] == '.')
        {
            out.push_back(atom);
        }

        // Function
        else if (isalpha((unsigned char)atom[0])) {
            ops.push(atom);
        }
        // Function argument separator
        else if (atom == ",") {
            while (!ops.empty() && ops.top() != "(") {
                out.push_back(ops.top()); ops.pop();
            }
        }
        // Operator
        else if (isOperator(atom)) {
            while (!ops.empty() && isOperator(ops.top())) {
                int topPr = getPriority(ops.top());
                int curPr = getPriority(atom);
                // ^ is right-associative
                if (topPr > curPr || (topPr == curPr && atom != "^")) {
                    out.push_back(ops.top()); ops.pop();
                } else break;
            }
            ops.push(atom);
        }

       //opening parenthesis
        else if (atom == "(") {
            ops.push(atom);
        }

        // closing parenthesis
        else if (atom == ")") {
            while (!ops.empty() && ops.top() != "(") {
                out.push_back(ops.top()); ops.pop();
            }
            if (ops.empty()) throw runtime_error("Missing '('");
            ops.pop(); 

            if (!ops.empty() && isalpha((unsigned char)ops.top()[0])) {
                out.push_back(ops.top()); ops.pop();
            }
        }
        else throw runtime_error("Unknown atom: " + atom);
    }

    // clear remaining operators
    while (!ops.empty()) {
        if (ops.top() == "(") throw runtime_error("miss '(' or ')'");
        out.push_back(ops.top()); ops.pop();
    }
    return out;
}

    //calculate RPN 
double evalRPN(const vector<string>& rpn) {
    stack<double> st;  

    for (auto& atom : rpn) {
        // Number
        if (isdigit((unsigned char)atom[0]) || atom[0] == '.')
        {
            st.push(stod(atom));
        }

        // Operator
        else if (isOperator(atom)) {
            if (st.size() < 2) throw runtime_error("Not enough operands");
            double b = st.top(); st.pop();
            double a = st.top(); st.pop();
            if (atom == "+") st.push(a + b);
            else if (atom == "-") st.push(a - b);
            else if (atom == "*") st.push(a * b);
            else if (atom == "/") {
                if (b == 0) throw runtime_error("Division by zero");
                st.push(a / b);
            }
            else if (atom == "^") st.push(pow(a, b));
        }
        
        // Function pow
        else if (atom == "pow") {
            if (st.size() < 2) throw runtime_error("pow requires two args");
            double b = st.top(); st.pop();
            double a = st.top(); st.pop();
            st.push(pow(a, b));
        }
        else throw runtime_error("Unknown atom: " + atom);
    }
    if (st.size() != 1) throw runtime_error("Evaluation error");
    return st.top();
}

// 
void process(const string& expr) {
    cout << "Expr: " << expr << "\n";
    try {
        auto atoms = atomize(expr);   
        auto rpn = toRPN(atoms);       

        cout << "RPN: ";
        for (auto& t : rpn) cout << t << " ";
        cout << "\n";

        cout << "Result = " << evalRPN(rpn) << "\n";
    } catch (exception& e) {
        cout << "Error: " << e.what() << "\n";
    }
    cout << "-------------------\n";
}




//   testiki
void runTests() {
    vector<string> tests = {
        "1+2", "2*3", "(1+2)",
        "pow(2,3)", "pow(pow(2,3),2)", "1+2*(3-4)",
        "(12.5+7)/3", "((1+2)", "1/0",
        "3.14+0.2", "5+", "2^3^2", "10+abc"
    };


    for (auto& e : tests) process(e);
}







int main() {
    cout << "1 - Enter expression\n2 - Run tests\nChoice: ";
    int c; 
    cin >> c; 
    cin.ignore();


    if (c == 1) { 
        string e; 
        getline(cin,e); 
        process(e); 
    }
    else if (c == 2) runTests();

    else cout << "Invalid choice";


}
