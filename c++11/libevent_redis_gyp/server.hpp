#pragma once

#include <sys/types.h>

#include <exception>

#include <iostream>
#include <cstdlib>
#include <csignal>

#include <event.h>
#include <evhttp.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include "hiredis.h"
#include "adapters/libevent.h"

using namespace std;

struct sib_config;

struct sib_cb {
    int status;
    string content_type;
    string body;
    struct sib_config *cfg;

    sib_cb() {}
    sib_cb(int status, 
           string content_type, 
           string body, 
           struct sib_config *cfg) : status(status), 
                                     content_type(content_type), 
                                     body(body),
                                     cfg(cfg) {}
    sib_cb(const sib_cb& cb) : status(cb.status), 
                               content_type(cb.content_type), 
                               body(cb.body),
                               cfg(cb.cfg) {}
};

enum class CType {
    REDIS_CONN_UNIX,
    REDIS_CONN_TCP
};

struct sib_config {
    struct {
        const char *host;
        int port;
        int timeout_secs;
        const char *content_type;
    } http;

    struct {
        enum CType type;
        int timeout_secs;
    } redis_conn;
    
    struct {
        const char *host;
        int port;
    } redis_tcp;

    struct {
        const char *path;
    } redis_unix;

    struct {
        void (*exec)(struct evhttp_request *, 
                     redisContext *, 
                     sib_cb&);
        void (*close)(struct sib_config *);
        struct evhttp_request *req;
        sib_cb res;
    } cb;
};

namespace sib {
    void request_msg(struct evhttp_request *req, int code, const char *tag, struct evbuffer *buffer, const char* msg) {
        if (NULL != msg) {
            evbuffer_add_printf(buffer, "%s", msg);   
        }
        evhttp_send_reply(req, code, tag, buffer);
    }

    void request_func(struct evhttp_request *req, void *arg) {        
        auto *cfg = static_cast<struct sib_config *>(arg);
        cfg->cb.req = req;

        auto ccb = [] (struct evhttp_connection *, void *arg) {
            auto config = static_cast<struct sib_config *>(arg);              
            (*config->cb.close)(config);
        };

        evhttp_connection_set_closecb(req->evcon, ccb, cfg);        
        
        auto *returnbuffer = evbuffer_new();
        
        redisContext *c = NULL;
        redisReply *r = NULL;

        try {      
            evhttp_add_header(req->output_headers, "Content-Type", cfg->http.content_type);

            if (cfg->redis_conn.type == CType::REDIS_CONN_TCP) {
                c = redisConnect(cfg->redis_tcp.host, cfg->redis_tcp.port);
            } else if (cfg->redis_conn.type == CType::REDIS_CONN_UNIX) {
                c = redisConnectUnix(cfg->redis_unix.path);
            }

            if (c->err) {
                request_msg(req, HTTP_SERVUNAVAIL, "ERROR", returnbuffer, "SERVICE UNAVAILABLE");
            }
            else {
                const struct timeval tv = { cfg->redis_conn.timeout_secs, 0 };

                if (REDIS_OK == redisSetTimeout(c, tv)) {
                    sib_cb cb(HTTP_INTERNAL, string(cfg->http.content_type), string("INTERNAL ERROR"), cfg);
          
                    auto exec = cfg->cb.exec;

                    exec(req, c, cb);

                    if (cb.status == HTTP_OK) {
                        cfg->cb.res = cb;
                        request_msg(req, HTTP_OK, "OK", returnbuffer, cb.body.c_str());
                    }
                    else {
                        request_msg(req, cb.status, "ERROR", returnbuffer, cb.body.c_str());
                    }
                }
                else {
                    request_msg(req, HTTP_INTERNAL, "ERROR", returnbuffer, "SET-TIMEOUT ERROR");
                }
            }
        }
        catch (exception& e) {
            request_msg(req, HTTP_INTERNAL, "ERROR", returnbuffer, e.what());
        }
        catch (...) {
            request_msg(req, HTTP_INTERNAL, "ERROR", returnbuffer, NULL);
        }

        evbuffer_free(returnbuffer);

        if (NULL != r) {
            freeReplyObject(r);   
        }
            
        if (NULL != c) {
            redisFree(c);
        }  
    }

    void request_handler(struct evhttp_request *req, void *arg) {
        request_func(req, arg);
    }

    int getCPUs()
    {
        return sysconf(_SC_NPROCESSORS_ONLN);
    }

    int setCPU(int cpu) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        if (sched_setaffinity(0, sizeof(cpu_set_t), &set) == 0) {
            return cpu;
        }
        return -1;
    }

    int getCPU() {
        int cpus = getCPUs();
        cpu_set_t set;
        CPU_ZERO(&set);
        if (sched_getaffinity(0, sizeof(cpu_set_t), &set) == 0) {
            for (int i = 0; i < cpus; i++) {
              if (CPU_ISSET(i, &set) > 0) {
                return i;
              }
            }
        }
        return -1;
    }

    void event_loop(sib_config *cfg) {
        auto *base = event_init();
        struct evhttp *http_server = evhttp_start(cfg->http.host, cfg->http.port);   
        evhttp_set_timeout (http_server, cfg->http.timeout_secs);     
        evhttp_set_gencb(http_server, sib::request_handler, cfg);

        cout << "server started on " << cfg->http.host << ":" << cfg->http.port << endl;
        
        setCPU(cfg->http.port % getCPUs());

        event_base_dispatch(base);
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
