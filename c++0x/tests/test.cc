#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

struct STRS {
    string x;
    string y;
    STRS(string a, string b) : x(a), y(b) {};
};

int main(int argc, char **argv) {

    vector<STRS> mystrs = { STRS("ivan ribeiro rocha", "alessandra cristina dos santos"),
                            {"babi luma lara", "babi luma lara lisa"} };

    for (STRS s : mystrs) {
        string &x = s.x;
        string &y = s.y;

        cout << "testing:\n\tx=" << x << "\n\ty=" << y;

        cout << "\n\n\tMAPS\n";

        map<char, bool> my;
        for (char c : y) {
            my[c] = true;
        }

        bool ok = true;
        for (char c : x) {
            if (!my[c]) {
                ok = false;
                cout << "\t\tx: " << c << " is not in y\n";
            }
        }

        cout << "\t\tres: y " << ((ok) ? "contains x" : "do not contains x") << "\n\n";

        cout << "\tSETS\n";

        auto comp = [](char a, char b){ return a < b; };
        set<char,decltype(comp)> sy(comp);
        for (char c : y) {
            sy.insert(c);
        }

        ok = true;
        for (char c : x) {
            if (sy.find(c) == sy.end()) {
                ok = false;
                cout << "\t\tx: " << c << " is not in y\n";                
            }
        }

        cout << "\t\tres: y " << ((ok) ? "contains x" : "do not contains x") << "\n\n";
    }
}