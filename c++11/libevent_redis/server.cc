#include "server.hpp"

// scons -c && scons && valgrind --leak-check=full ./server
// redis-cli -s /tmp/redis.sock get 8081
// httperf --server localhost --port 8081 --uri / --num-call 100 --num-conn 100

void func(struct evhttp_request *req, redisContext *c, sib_cb& cb) {
    redisReply *r = NULL;

    try {
        r = static_cast<redisReply *>(redisCommand(c, 
            "INCR %d", cb.cfg->http.port));

        if (NULL != r && r->type == REDIS_REPLY_INTEGER) {  
                cb.status = HTTP_OK;
                cb.body = string_format("(cpu:%d) request [%s] from [%s:%d] = %d", sib::getCPU(),
                    req->uri, req->remote_host, req->remote_port, r->integer);
        } 
        else {
                cb.body = string("REDIS_REPLY_ERROR");
        }
    }
    catch(...) {
        cb.status = HTTP_INTERNAL;
        cb.body = string("REDIS_ERROR");
    }

    if (NULL != r) {
        freeReplyObject(r);
    }
}

void close(struct sib_config * cfg) {
    ;
}

int main()
{
/*
    struct sib_config cfg = { { "127.0.0.1", 8081, 5, "application/json; charset=UTF-8" },
                              { CType::REDIS_CONN_UNIX, 5 }, 
                              { "localhost", 6379 }, 
                              { "/tmp/redis.sock" }, 
                              { &func, &close, NULL, {} } };
*/                              
    struct sib_config cfg = { { "127.0.0.1", 8081, 5, "application/json; charset=UTF-8" },
                              { CType::REDIS_CONN_TCP, 5 }, 
                              { "localhost", 6379 }, 
                              { "" }, 
                              { &func, &close, NULL, {} } };

    sib::event_loop(&cfg);

    return 0;
}
