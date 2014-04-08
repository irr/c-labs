#include <stdio.h>
#include <event.h>
#include <evhttp.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

/* 
gcc -o http http.o -lpthread -levent -lrt
http localhost:8081/test
*/

#define PORT    8081
#define THREADS   10

void process_request(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    
    if (buf == NULL) return;

    evbuffer_add_printf(buf, "Requested: %s\n", evhttp_request_uri(req));
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

static void* thread_dispatch(void *arg) {
    event_base_dispatch((struct event_base*) arg);
    return NULL;
}

int main () {

    int nfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfd < 0) return -1;

    int one = 1;
    int r = setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int));
    if (r < 0) return -1;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    r = bind(nfd, (struct sockaddr*)&addr, sizeof(addr));
    if (r < 0) return -1;

    r = listen(nfd, 10240);
    if (r < 0) return -1;

    int flags = fcntl(nfd, F_GETFL, 0);
    if ((flags < 0)
          || (fcntl(nfd, F_SETFL, flags | O_NONBLOCK) < 0))
        return -1;

    pthread_t ths[THREADS];

    int i;
    for (i = 0; i < THREADS; i++) {
        struct event_base *base = event_init();
        if (base == NULL) return -1;

        struct evhttp *httpd = evhttp_new(base);
        if (httpd == NULL) return -1;

        r = evhttp_accept_socket(httpd, nfd);
        if (r != 0) return -1;

        evhttp_set_gencb(httpd, process_request, NULL);

        r = pthread_create(&ths[i], NULL, &thread_dispatch, base);
        if (r != 0) return -1;
    }

    for (i = 0; i < THREADS; i++) {
        pthread_join(ths[i], NULL);
    }

    return 0;
}