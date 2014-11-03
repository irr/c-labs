#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory> 
#include <regex>
#include <string>
#include <vector>

#include <iconv.h>

// valgrind --show-reachable=yes --trace-children=yes --leak-check=full ./srt <srt file>

using namespace std;

inline string format(const char* fmt, ...){
    int size = 512;
    char* buffer = 0;
    buffer = new char[size];
    va_list vl;
    va_start(vl, fmt);
    int nsize = vsnprintf(buffer, size, fmt, vl);
    if(size<=nsize){ //fail delete buffer and try again
        delete[] buffer;
        buffer = 0;
        buffer = new char[nsize+1]; //+1 for /0
        nsize = vsnprintf(buffer, size, fmt, vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete[] buffer;
    return ret;
}

void shell(const std::string &cmd, std::string &out) {
    FILE *fp = nullptr;
    char buff[4096];
    try {
        fp = popen(cmd.c_str(), "r");
        if (fp != NULL) {
            while (fgets(buff, sizeof(buff), fp)!=NULL){
                out += string(buff);
            }   
            pclose(fp);
        }
    } catch (...) {
        if (fp) pclose(fp);
    }
}

void read_file(const string &file, string &content) {
    ifstream ifs(file);
    try {
        string txt((std::istreambuf_iterator<char>(ifs)),
                   (std::istreambuf_iterator<char>()));
        content = txt;
    } catch (const exception& _) {
        throw;
    }
}

void write_file(const string &file, string &content) {
    ofstream ofs(file, ios::trunc);  
    try {
        ofs << content;
    } catch (const exception& _) {
        throw;
    }
}

bool toutf8(const string &file, string &charset) {
    iconv_t conv = nullptr;
    try {
        string content;
        read_file(file, content);
        size_t dstlen = content.length() * 2;
        const char *src = content.c_str();
        size_t srclen = content.length();
        char dst[dstlen];
        char *pin = (char*) src;
        char *pout = (char*) dst;
        conv = iconv_open("UTF-8//TRANSLIT", charset.c_str());
        iconv(conv, (char **) &pin, &srclen, &pout, &dstlen);
        iconv_close(conv);
        string out = string(dst);
        write_file(file, out);
        return true;
    } catch (const exception& _) {
        if (conv) iconv_close(conv);
        return false;
    }

}
int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: srt <file>" << endl;
        return 1;
    }
    vector<std::string> arguments(argv, argv + argc);
    string file = arguments[1];
    
    string cmd = format("file -bi \"%s\"", file.c_str());
    string out = "";
    shell(cmd, out);
    if (out.length() == 0) return 1;
    regex r("charset=(.*)");    
    smatch m;
    string charset = "";    
    if (regex_search(out, m, r)) {
        charset = m.str().substr(8);
    }
    if (charset.length() == 0) return 1;
    transform(charset.begin(), charset.end(), charset.begin(), ::tolower);
    if (charset != "utf-8") {
        if (!toutf8(file, charset)) return 1;
    }

    string content = "";
    read_file(file, content);
    regex rp("<[^>]+>");
    content = regex_replace(content, rp, "");
    write_file(file, content);

    system(format("zenity --info --text='Filename: %s\\nEncoding: [%s]'", file.c_str(), charset.c_str()).c_str());
    return 0;
}