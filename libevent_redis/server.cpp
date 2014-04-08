#include <sys/types.h>

#include <iostream>
#include <cstdlib>
#include <csignal>
#include <pthread.h>

#include <event.h>
#include <evhttp.h>

#include "hiredis.h"
#include "async.h"
#include "adapters/libevent.h"

redisAsyncContext *c;

void request_handler(struct evhttp_request *req, void *arg)
{
    std::string value(req->remote_host);
    redisAsyncCommand(c, NULL, NULL, "SET rclient %b", (char*) value.c_str(), value.length());

    struct evbuffer *returnbuffer = evbuffer_new();

    evbuffer_add_printf(returnbuffer, "OK!");
    evhttp_send_reply(req, HTTP_OK, "OK", returnbuffer);
    evbuffer_free(returnbuffer);

    return;
}

void get_callback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply*) r;
    if (reply == NULL) {
        std::cerr << "Error: no data" << std::endl;
        return;
    }
    std::cout << "data: " << (char*)privdata << " and reply: " << reply->str << std::endl;
}

void connect_callback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        std::cerr << "Error: " << c->errstr << std::endl;
        return;
    }
    std::cout << "redis connected..." << std::endl;
}

void disconnect_callback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        std::cerr << "Error: " << c->errstr << std::endl;
        return;
    }
    std::cout << "redis disconnected..." << std::endl;
}

void *redis_func(void *p) {
	redisAsyncContext *c = (redisAsyncContext *) p;
    struct event_base *redis = event_base_new();

    redisLibeventAttach(c, redis);
    redisAsyncSetConnectCallback(c, connect_callback);
    redisAsyncSetDisconnectCallback(c, disconnect_callback);

    std::cout << "redis client started..." << std::endl;

    event_base_dispatch(redis);
}

int main(int argc, char **argv)
{
    short          http_port = 8081;
    char           http_addr[] = "127.0.0.1";
    struct evhttp *http_server = NULL;
    pthread_t      t;

    event_init();

    http_server = evhttp_start(http_addr, http_port);
    evhttp_set_gencb(http_server, request_handler, NULL);

    c = redisAsyncConnect("127.0.0.1", 6379);

    pthread_create(&t, NULL, redis_func, c);

    std::cout << "server started on port " << http_port << std::endl;

    event_dispatch();

    pthread_join(t, NULL);

    return 0;
}
