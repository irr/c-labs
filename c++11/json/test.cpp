#include <iostream>
#include <string>
#include <sstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

/*
{
    "path1":
    {
        "path2":
        [
            {
                "int": "10",
                "bool": "true"
            },
            {
                "double": "2.2",
                "string": "\"some string\""
            }
        ]
    }
}
 */

int main(int, char *[])
{
    using namespace std;
    using boost::property_tree::ptree;
    using boost::property_tree::basic_ptree;
    try {
        ptree root, arr,elem2;
        basic_ptree<std::string, std::string> elem1;
        elem1.put<int>("int", 10 );
        elem1.put<bool>("bool", true);
        elem2.put<double>("double", 2.2);
        elem2.put<std::string>("string", "some string");

        arr.push_back( std::make_pair("", elem1) );
        arr.push_back( std::make_pair("", elem2) );
        root.put_child("path1.path2", arr);

        std::stringstream ss;
        write_json(ss, root);

        std::string my_string_to_send_somewhere_else = ss.str();
        cout << my_string_to_send_somewhere_else << endl;
    }
    catch (std::exception & e) {
        cout << e.what();
    }
    return 0;
}

