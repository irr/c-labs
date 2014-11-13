#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <Wt/WResource>

using namespace Wt;
using namespace Wt::Http;

class Resource: public WResource {

    public:
        Resource();
        virtual ~Resource();

    protected:
        virtual void handleRequest(const Request &request, Response &response);
};

#endif /* RESOURCE_H_ */
