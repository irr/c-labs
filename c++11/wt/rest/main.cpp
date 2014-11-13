#include <Wt/WServer>

#include "resource.hpp"
#include "log.hpp"

// ./server --http-address 0.0.0.0 --http-port 8080 --docroot=.
// curl -X POST -H "Content-Type: application/json" -d "{\"test\":10}" http://localhost:8080/

using namespace std;
using namespace Wt;

int main(int argc, char **argv) {
    WLogger logger;
    //configLogger(logger);
    try {
        WServer server(argv[0], "");
        try {
            server.setServerConfiguration(argc, argv);
            Resource dr;
            server.addResource(&dr, "/");
            info(logger, "Starting server.");
            if (server.start()) {
                WServer::waitForShutdown();
                server.stop();
            } else {
                fatal(logger, "Fatal error starting server.");
                return 1;
            }
            return 0;
        } catch (std::exception& e) {
            fatal(logger, "Fatal error starting server.", e.what());
            return 1;
        }
    } catch (WServer::Exception& e) {
        fatal(logger, "Fatal error creating WServer.", e.what());
        return 1;
    } catch (exception& e) {
        fatal(logger, "Fatal error occurred.", e.what());
        return 1;
    }
}

