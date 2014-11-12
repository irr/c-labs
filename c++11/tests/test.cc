#include <algorithm>
#include <cassert>
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

struct node {
    int k;
    node *pl;
    node *pr;
};    

void debug(node* n, const char* s = nullptr) {
    if (s != nullptr) {
        cout << "  [" << setw(4) << n->k << "] " << setw(24) << s << ", p: " << setw(10) << n 
             << ", l:" << setw(10) << n->pl << ", r:" << setw(10) << n->pr << endl;    
    } else {
        cout << "  [" << setw(4) << n->k << "] " << "p: " << setw(10) << n 
             << ", l:" << setw(10) << n->pl << ", r:" << setw(10) << n->pr << endl;    
    }
}
node* insert(node *p, int k) {
    if (p == nullptr) {
        node* n = new node;
        n->k = k;
        n->pl = nullptr;
        n->pr = nullptr;
        debug(n, "return n");
        cout << endl;
        return n;
    }
    if (k < p->k) {
        debug(p, "1.(k < p->k)");
        p->pl = insert(p->pl, k);
        debug(p, "2.(k < p->k)");
    } else {
        debug(p, "1.(k >= p->k)");
        p->pr = insert(p->pr, k);
        debug(p, "2.(k >= p->k)");
    }
    debug(p, "return p");
    cout << endl;
    return p;
}

node* search(node* p, int k) {
    if (p == nullptr) {
        return nullptr;
    } else if (p->k == k) {
        debug(p, "return p");
        return p;
    } else if (k < p->k) {
        debug(p, "(k < p->k)");
        return search(p->pl, k);
    } else {
        debug(p, "(k >= p->k)");
        return search(p->pr, k);
    }
}

void inorder(node* p) {
    if (p == nullptr) {
        return;
    }
    inorder(p->pl);
    debug(p, "inorder");
    inorder(p->pr);
}

void destroy(node* p) {
    if (p != nullptr) {
        destroy(p->pl);
        destroy(p->pr);
        debug(p, "deleting...");
        delete p;
    }
}

node* findmax(node* p) {
    if (p == nullptr) {
        return nullptr;
    }
    if (p->pr == nullptr) {
        return p;
    }
    return findmax(p->pr);
}

node* removemax(node* p, node* pm) {
    if (p == nullptr) {
        return nullptr;
    }    
    if (p == pm) {
        return pm->pl;
    }
    debug(p, "1.removemax(pm)");
    p->pr = removemax(p->pr, pm);
    debug(p, "2.removemax(pm)");
    return p;
}

node* remove(node* p, int k) {
    if (p == nullptr) {
        return nullptr;
    }
    if (p->k == k) {
        if (p->pl == nullptr) {
            node* prs = p->pr;
            debug(p, "(p->pl == nullptr)");
            delete p;            
            return prs;
        }
        if (p->pr == nullptr) {
            node* pls = p->pl;
            debug(p, "(p->pr == nullptr)");
            delete p;
            return pls;
        }
        node* pm = findmax(p->pl);
        debug(pm, "findmax(p->pl)");
        pm->pl = removemax(p->pl, pm);
        pm->pr = p->pr;
        debug(p, "removed");
        delete p;
        return pm;
    } else if (k < p->k) {
        p->pl = remove(p->pl, k);
    } else if (k > p->k) {
        p->pr = remove(p->pr, k);
    }
    return p;
}

int main(int argc, char **argv) {

    vector<STRS> mystrs = { STRS("ivan rocha", "alessandra santos"),
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

    vector <unsigned int> vn = {13, 1984, 3999};
    
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

    unsigned int x, n = 19720403;

    cout << "building string from n = " << n << " ... ";
    
    string s;
    while (n > 0) {
        x = n % 10;
        n /= 10;
        s.append(1, (char)('0' + x));
        cout << x << " ";
    }

    std::reverse(s.begin(), s.end());
    cout << "string: \"" << s << "\"\n\n";

    node* root = nullptr;
    // https://www.cs.usfca.edu/~galles/visualization/BST.html
    for (int i : {5, 10, 2, 7, 13, 1, 3, 4, 0}) {
        cout << "insert  " << setw(2) << i << endl;
        root = insert(root, i);
    }
    cout << "root: " << root << endl;

    inorder(root);

    cout << "\nsearch: 7" << endl;
    assert(search(root, 7) != nullptr);
 
    cout << "\nremoving: 5" << endl;
    root = remove(root, 5);

    inorder(root);

    cout << "\ndestroying..." << endl;
    destroy(root);

    cout << endl;
}