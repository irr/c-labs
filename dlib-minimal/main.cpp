#include <dlib/logger.h>
#include <dlib/cmd_line_parser.h>

using namespace dlib;

typedef cmd_line_parser<char>::check_1a_c clp;

logger dlog("minimal");

int main(int argc, char **argv)
{
    clp parser;

    dlog.set_level(LALL);
    parser.add_option("t", "test option");
    parser.parse(argc,argv);

    if (parser.option("t"))
        dlog << LINFO << "test option ok";

	dlog << LINFO << "main ok";

    return 0;
}
