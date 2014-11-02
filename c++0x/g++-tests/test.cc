// g++ -std=c++0x -pthread test.cc -o test && ./test

#include <iostream>
#include <thread>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>
#include <cstddef>
#include <cstdint>

using namespace std;

void thread_func()
{
    cout<<"hello thread!"<<std::endl;
}


int &fint() {
    static int value;
    cout << "\tfint: " << value << endl;
    return value;
}

void rvalue(int &value) {
    cout << "\treference &: " << value << endl;
}

void rvalue(int &&value) {
    ++value;
    cout << "\trvalue &&: " << value << endl;
}

int intVal() {
    return 42;
}

struct Data
{
    char *text;
    size_t size;

    void copy(Data const &other) {
        text = strdup(other.text);
        size = strlen(text);
    }

    void copy(Data &&other)
    {
        text = other.text;
        other.text = 0;
    }

    void free() {
        ::free(text);
    }
};

Data dataFactory(char const *txt)
{
    Data ret = {strdup(txt), strlen(txt)};
    return ret;
}

enum class UEnum : unsigned char {
    NOT_OK, // 0, by implication
        OK = 10,
        MAYBE_OK // 11, by implication
        };

template<typename T>
void listAll(typename std::initializer_list<T> values) {
    cout << "\t{ ";
    //for (typename std::initializer_list<T>::const_iterator it = values.begin(); it != values.end(); ++it)
    for (auto it = values.begin(); it != values.end(); ++it)
        cout << *it << " ";
    cout << "}" << endl;
}

int sum(int x, int y) {
    return x + y;
}

int isum(decltype(sum) f, int x, int y) {
    return (*f)(x, y);
}

int main() {
    // 64-bit only: static_assert((sizeof(long) >= 8), "64-bit code generation is not enabled/supported.");

    cout << endl << "Initiating c++0x tests..." << endl << endl;

    cout << "> testing rvalues..." << endl;
    cout << "\tcalling fint() = 20..." << endl;
    fint() = 20;

    cout << "\tcalling fint() += fint()..." << endl;
    fint() += fint();

    cout << "\tcalling fint()..." << endl;
    fint();

    cout << "\tcalling rvalue(41)..." << endl;
    rvalue(41);

    int x(10);
    cout << "\tcalling rvalue(x) where x = 10..." << endl;
    rvalue(x);

    cout << "\tcalling rvalue(intVal()) where intVal() returns 42..." << endl;
    rvalue(intVal());
    cout << endl;

    cout << "> testing structs and rvalues..." << endl;

    Data d1 = { strdup("ivan ribeiro rocha"), strlen("ivan ribeiro rocha") };
    Data d2;
    d2.copy(d1);

    Data d3;
    d3.copy(dataFactory("ivan.ribeiro@gmail.com"));

    cout << "\td1: " << d1.text << " @" << &d1.text << endl;
    cout << "\td2: " << d2.text << " @" << &d2.text << endl;
    cout << "\td3: " << d3.text << " @" << &d3.text << endl << endl;

    d1.free();
    d2.free();
    d3.free();

    const char * const prs = R"(\w\\\w)";
    const string srs = string(R"([Another \ Raw \n\r\t(String)])");

    cout << "> testing raw strings with delimiters=()..." << endl;
    cout << "\t" << prs << endl;
    cout << "\t" << srs << endl << endl;

    cout << "> testing strongly typed enumerations..." << endl;
    cout << "\tsizeof UEnum : unsigned char is " << sizeof UEnum::MAYBE_OK << endl << endl;

    cout << "> testing initializer lists..." << endl;
    listAll({1,2,3,4,5,10,11,42});
    listAll({"a1", "b1", "c1", "a2", "b2", "c2"});
    listAll({10.1,20.2,30.3,40.4,50.5,100.01,420.42});
    cout << endl;

    cout << "> testing auto and decltype with function pointers..." << endl;
    int (*add)(int,int) = sum;
    auto aadd = sum;
    cout << "\tusing [int (*add)(int,int) = sum] passing 1 and 2: " << (*add)(1,2) << endl;
    cout << "\tusing [auto aadd = sum] passing 1 and 2: " << aadd(1,2) << endl;
    cout << "\tusing [int isum(decltype(sum) f, int x, int y)] passing 1 and 2: " << isum(sum,1,2) << endl << endl;

    /*
      []      // no variables defined. Using one will result in error
      [x, &y] // x is captured by value, y is captured by reference
      [&]     // any external variable is implicitly captured by reference if used
      [=]     // any external variable is implicitly captured by value if used
      [&, x]  // x is explicitly captured by value. Other variables will be captured by reference
      [=, &z] // z is explicitly captured by reference. Other variables will be captured by value
    */
    cout << "> testing lambda functions and expressions (closure) using [&, value](int num) ..." << endl;
    std::vector<int> nums =  std::initializer_list<int>({1,2,3,4,5});

    int total = 0;
    int value = 10;
    std::for_each(nums.begin(), nums.end(), [&, value](int num) {
            total += num * value;
            cout << "\t step: {total:" << setw(4) << total << "}, {num:"
                 << setw(2) << num << "}, {value:"
                 << setw(3) << value << "}" << endl;
        });
    cout << "\t end:  {total:" << setw(4) << total << "}, {value:" << setw(3) << value << "}" << endl << endl;

    auto sum4 = [=](int n) -> int { return (n + 4 + total); };
    auto sum7 = [=](int n) -> int { return (n + 7); };

    cout << "\t (closure) sum4 of 10 is " << (sum4)(10) << endl;

    decltype(sum7) psum7 = sum7;
    cout << "\t (closure) psum7 of 10 is " << (*psum7)(10) << endl;

    char utf8[] = u8"This is UTF-8 encoded.";
    char16_t utf16[] = u"This is UTF-16 encoded.";
    char32_t utf32[] = U"This is UTF-32 encoded.";

    string s8 = string(utf8);
    u16string s16 = u16string(utf16);
    u32string s32 = u32string(utf32);

    cout << "> testing utfs and long/int types..." << endl;
    cout << "\t UTF-8: " << utf8 << " (" << s8.length() << ")" << endl;
    cout << "\t UTF-16 and 32 sizes: (" << s16.length() << ", " << s32.length() << ")" << endl;

    long long int i64 = 100;
    uint32_t u32 = 100;
    uint64_t u64 = 100;
    size_t st = 0;
    cout << "\t sizeof (long long int) is always " << sizeof i64 << " bytes in any platform" << endl;
    cout << "\t sizeof (uint32_t) is always " << sizeof u32 << " bytes in any platform" << endl;
    cout << "\t sizeof (uint64_t) is always " << sizeof u64 << " bytes in any platform" << endl;
    cout << "\t sizeof (size_t) is " << sizeof st << " bytes" << endl << endl;

    cout << "> testing const and reinterpret casts..." << endl;

    char const hello[] = "xello";
    const_cast<char *>(hello)[0] = 'h';
    cout << "\t changing [char const hello = 'xello'] to " << hello << " using const_cast<T>" << endl;

    uint32_t h = 0x12345678;
    cout << "\t 0x12345678 first byte (intel=little-endian) has value: " << hex <<
        static_cast<uint32_t>(
                              *reinterpret_cast<unsigned char *>(&h)
                              ) << " using reinterpret_cast<T>" << endl << endl;

	thread t(thread_func);
	t.join();
}
