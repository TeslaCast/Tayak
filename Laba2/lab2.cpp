#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <stdexcept>
#include <string>

using namespace std;


using TransitionTable = map<pair<string, char>, set<string>>;

class Automat {
public:
    TransitionTable transitions;
    string startState = "q0";
    set<string> finalStates;
    set<string> allStates;

    void loadFromFile(const string& filename) {
        ifstream fin(filename);
        if (!fin.is_open()) throw runtime_error("Не удалось открыть файл");

        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            size_t commaPos = line.find(',');
            size_t eqPos = line.find('=');
            if (commaPos == string::npos || eqPos == string::npos)
                throw runtime_error("Ошибка формата");

            string fromState = line.substr(0, commaPos);
            char symbol = line[commaPos + 1];
            string toState = line.substr(eqPos + 1);

            bool isFinal = (toState[0] == 'f' || toState[0] == 'F');
            if (isFinal) {
                string norm = "q" + toState.substr(1);
                finalStates.insert(norm);
                toState = norm;
            }

            allStates.insert(fromState);
            allStates.insert(toState);
            transitions[{fromState, symbol}].insert(toState);
        }
    }

    //детерминирован ли автомат
    bool isDeterministic() const {
        for (auto& [key, nextStates] : transitions) {
            if (nextStates.size() > 1) return false;
        }
        return true;
    }

    //  детерминирование
    Automat determinize() const {
        Automat dfa;
        dfa.startState = startState;

        queue<set<string>> q;
        map<set<string>, string> stateName;
        int counter = 0;

        set<string> startSet = { startState };
        stateName[startSet] = startState;
        q.push(startSet);
        dfa.allStates.insert(startState);

        while (!q.empty()) {
            set<string> curSet = q.front(); q.pop();
            string curName = stateName[curSet];

            for (auto& s : curSet)
                if (finalStates.count(s)) dfa.finalStates.insert(curName);

            map<char, set<string>> grouped;

            for (auto& s : curSet) {
                for (auto& [key, nextStates] : transitions) {
                    if (key.first == s) {
                        grouped[key.second].insert(nextStates.begin(), nextStates.end());
                    }
                }
            }

            for (auto& [sym, nextSet] : grouped) {
                if (!stateName.count(nextSet)) {
                    string newName = "Q" + to_string(++counter);
                    stateName[nextSet] = newName;
                    dfa.allStates.insert(newName);
                    q.push(nextSet);
                }
                string nextName = stateName[nextSet];
                dfa.transitions[{curName, sym}].insert(nextName);
            }
        }
        return dfa;
    }

    // проверка строки
    bool accepts(const string& input) const {
        if (!isDeterministic())
            throw runtime_error("Автомат недетерминирован. Сначала преобразуйте в ДКА.");
        string curState = startState;
        for (char c : input) {
            auto it = transitions.find({curState, c});
            if (it == transitions.end()) return false;
            curState = *it->second.begin();
        }
        return finalStates.count(curState);
    }


    void print() const {
        for (auto& [key, nextStates] : transitions) {
            for (auto& st : nextStates) {
                bool fin = finalStates.count(st);
                cout << key.first << "," << key.second << "="
                     << (fin ? "f" : "q") << st.substr(1) << "\n";
            }
        }
    }
};


void generateAcceptedStrings(const Automat& automaton, int maxLen) {
    set<char> alphabet;
    for (auto& [key, _] : automaton.transitions)
        alphabet.insert(key.second);

    cout << "\nВсе принимаемые строки длиной ≤ " << maxLen << ":\n";

    queue<pair<string,string>> q; 
    q.push({automaton.startState, ""});

    while (!q.empty()) {
        auto [state, word] = q.front(); q.pop();

        if (automaton.finalStates.count(state))
            cout << (word.empty() ? "ε" : word) << "\n";

        if ((int)word.size() >= maxLen) continue;

        for (char sym : alphabet) {
            auto it = automaton.transitions.find({state, sym});
            if (it != automaton.transitions.end()) {
                string next = *it->second.begin();
                q.push({next, word + sym});
            }
        }
    }
}


void printGraph(const Automat& automaton) {
    cout << "\nГраф переходов:\n";
    for (auto& [key, nextStates] : automaton.transitions) {
        for (auto& st : nextStates) {
            cout << key.first << " --" << key.second << "--> " << st;
            if (automaton.finalStates.count(st)) cout << " [final]";
            cout << "\n";
        }
    }
}


int main() {
    setlocale(LC_ALL, "russian");
    try {
        Automat nfa;
        nfa.loadFromFile("var4.txt");

        cout << "Автомат загружен.\n";
        cout << "Детерминированный? " << (nfa.isDeterministic() ? "Да" : "Нет") << "\n";

        Automat dfa = nfa;
        if (!nfa.isDeterministic()) {
            cout << "\nПреобразование в ДКА...\n";
            dfa = nfa.determinize();
            cout << "Переходы ДКА:\n";
            dfa.print();
        }

        printGraph(dfa);
        generateAcceptedStrings(dfa, 8); 

        cout << "\nВведите строку для проверки: ";
        string input;
        cin >> input;

        bool res = dfa.accepts(input);
        cout << (res ? "Строка принимается автоматом\n" : "Строка не принимается автоматом\n");
        
    } catch (const exception& ex) {
        cerr << "Ошибка: " << ex.what() << "\n";
    }
    return 0;
}
