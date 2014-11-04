#include <iostream>
#include <string>
#include <vector>

// TCO: ./test 100000000

using namespace std;

unsigned int f(unsigned int a, unsigned int b) {
    if (a == 0) {
        return b;
    }
    return f(a-1, b);
}

int main(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    int n = stoi(args[1]); 
    std::cout << f(n, n) << ": OK\n";
}