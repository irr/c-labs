#include <Wt/Http/Response>

#include "resource.hpp"

using namespace std;
using namespace Wt::Http;

Resource::Resource() {
}

Resource::~Resource() {
}

void Resource::handleRequest(const Request& request, Response& response) {
    string method = request.method();
    string contentType = request.contentType();
    int contentLength = request.contentLength();
    char* buffer = new char[contentLength + 1];
    request.in().read(buffer, contentLength);
    buffer[contentLength] = 0;
    response.setMimeType("application/json");
    ostream& out = response.out();
    out << "{" << endl;
    out << "\t\"method\":\"" << method << "\"," << endl;
    out << "\t\"content-type\":\"" << contentType << "\"," << endl;
    out << "\t\"content-length\":\"" << contentLength << "\"," << endl;
    out << "\t\"body\":\"" << buffer << "\"" << endl;
    out << "}" << endl;
    delete[] buffer;
}

