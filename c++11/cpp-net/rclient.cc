#include <boost/network/protocol/http/server.hpp>
#include <string>
#include <iostream>
#include <csignal>
#include <cstdlib>

#include <unistd.h>
#include <pthread.h>

#include "hiredis.h"

namespace http = boost::network::http;

struct hello_world;
typedef http::server<hello_world> server;

struct hello_world {
    int idx;
    void operator() (server::request const &request,
                     server::response &response) {
        std::string ip = source(request);

        redisContext *c = NULL;
        redisReply *reply = NULL;

        try {
            c = redisConnectUnix("/tmp/redis.sock");

            if (c->err) {
                std::cerr << c->errstr << std::endl;
            }
            else {
                reply = (redisReply*) redisCommand(c, "INCR %d", idx);
                if (NULL != reply && reply->type == REDIS_REPLY_INTEGER) { 
                    response = server::response::stock_reply(server::response::ok, 
                        string_format("%s:%d = %d", ip.c_str(), idx, reply->integer));
                }
                else {
                    response = server::response::stock_reply(server::response::ok, 
                        string_format("%s:%d = REDIS_REPLY_ERROR", ip.c_str(), idx));
                }
            }
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

        try {
            if (NULL != reply) freeReplyObject(reply);
            if (NULL != c) redisFree(c);
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

    }
    std::string string_format(const std::string &fmt, ...) {
        int n, size = 256;
        std::string str;
        va_list ap;
        for (;;) {
            str.resize(size);
            va_start(ap, fmt);
            int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
            va_end(ap);
            if (n > -1 && n < size) return str;
            size = (n > -1) ? (n + 1) : size * 2;
        }
    }    
    void log(...) {
    }
};

int
main(int argc, char * argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " address port" << std::endl;
        return 1;
    }

    try {
        hello_world handler;   
        handler.idx = atoi(argv[2]);     
        server server_(argv[1], argv[2], handler);
        //server_.reuse_address(true);
        server_.run();
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }


    return 0;
}
