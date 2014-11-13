#include <Wt/Http/Response>

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "resource.hpp"

using namespace std;
using namespace Wt::Http;
using boost::property_tree::ptree; 
using boost::property_tree::read_json; 
using boost::property_tree::write_json;

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
    
    ptree pt;
    pt.put("method", method);
    pt.put("content-type", contentType);
    pt.put("content-length", contentLength);
    pt.put("body", buffer);
    
    ostringstream buf; 
    write_json(buf, pt, false);

    string json = buf.str();

    ptree pt2;
    istringstream is(json);
    read_json(is, pt2);
    string m = pt2.get<string>("method");
    pt2.put("check", m.length() > 0);

    buf.seekp(ios_base::beg);

    write_json(buf, pt2, false);
    out << buf.str() << endl;

    delete[] buffer;
}

