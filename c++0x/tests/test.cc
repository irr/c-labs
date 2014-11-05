#include <algorithm>
#include <iostream>
#include <iomanip>
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

    // http://www.wikihow.com/Learn-Roman-Numerals
    // MCMLXXXIV = 1984 (M=1000; CM=900; LXXX=80; IV=4)

    cout << "converting to roman-numerals:\n\n";

    map<int, string> mr = { {1, "I"},    {2, "II"},    {3, "III"}, {4, "IV"}, {5, "V"}, 
                            {6, "VI"},   {7, "VII"},   {8, "VII"}, {9, "IX"}, {10, "X"},
                            {20, "XX"},  {30, "XXX"},  {40, "XL"}, {50, "L"}, {60, "LX"},
                            {70, "LXX"}, {80, "LXXX"}, {90, "XC"}, 

                            {100, "C"},  {200, "CC"},  {300, "CCC"}, {400, "CD"},
                            {500, "D"},  {600, "DC"},  {700, "DCC"}, {800, "DCCC"}, 
                            {900, "CM"}, {1000, "M"}};

    vector<unsigned int> vd;

    for(auto it = mr.begin(); it != mr.end(); ++it) {
        vd.push_back(it->first);
    }
    auto vc = [](unsigned int a, unsigned int b){ return a > b; };
    sort(vd.begin(), vd.end(), vc);

    vector <unsigned int> vn = {1984, 3999};
    
    for (unsigned int n : vn) {
        cout << "\tN=" << n << endl;
        vector<string> x;
        unsigned int v = n;
        for (unsigned int d : vd) {
            unsigned int a = v / d;
            for (unsigned int i = 0; i < a; ++i) {
                x.push_back(mr[d]);
            }
            if (a > 0) {
                v = v - (a * d);
                cout << "\t\t" << setw(4) << d << ": " << a << " - " << v << endl;
            }
        }

        cout << "\tRoman-Numeral: ";
        for (string a : x) {
            cout << a;
        }

        cout << endl << endl;
    }

    unsigned int n = 19720403;
    unsigned int x;

    string s;

    cout << "building string from n = " << n << " ... ";

    while (n > 0) {
        x = n % 10;
        n = n / 10;
        s.append(1, (char)('0'+x));
        cout << x << " ";
    }

    std::reverse(s.begin(), s.end());
    cout << "string: \"" << s << "\"\n\n";
}