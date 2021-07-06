//
//  main.cpp
//  NetTest
//
//  Created by Knove on 2021/6/29.
//

#include <iostream>
#include <string.h>
#include <uv.h>
#include <nlohmann/json.hpp>

using namespace nlohmann;
using namespace std;

#define CHECK(r, msg)                                       \
    if (r<0) {                                              \
        fprintf(stderr, "%s: %s\n", msg, uv_strerror(r));   \
        exit(1);                                            \
    }


void sendInfo();
int status;
static uv_loop_t *uv_loop;
static uv_udp_t   server;
static uv_udp_t   send_server;

sockaddr_in addr;
uv_buf_t read_buf;

uint64_t bytes_send = 0;

static void on_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* rcvbuf, const struct sockaddr* addr, unsigned flags) {
    if (nread > 0) {
//        printf("%lu\n",nread);
        cout << "recv data:" << (string)rcvbuf->base << endl;
        if (((string)rcvbuf->base).compare("send") == true) {
            sendInfo();
        }
    }
    free(rcvbuf->base);
}
 
static void on_alloc(uv_handle_t* client, size_t suggested_size, uv_buf_t* buf) {
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
    
    bzero(buf->base, suggested_size);
//    printf("malloc:%lu %p\n",buf->len,buf->base);
}

//static void close_cb(uv_handle_t* handle)
//{
//    uv_is_closing(handle);
//}

void on_udp_send(uv_udp_send_t *req, int status) {
//    uv_close((uv_handle_t*) req->handle, close_cb);
    free(req);
    if (status) {
        fprintf(stderr, "Send error %s\n", uv_strerror(status));
        return;
    }
}

void sendInfo () {
    
    struct sockaddr_in send_addr;
    
    uv_ip4_addr("0.0.0.0", 7700, &send_addr);
//    uv_udp_connect(&server, (const struct sockaddr *)&send_addr);
    
    json j2 = {
      {"type", "1"},
      {"msg", "ping"},
    };
    string jsonStr = j2.dump();
    read_buf.base = (char*)j2.dump().data();
    read_buf.len = strlen(read_buf.base);
//    send_req.data = (void *)&read_buf;
    cout<<"send str: " << jsonStr << endl;
    
    uv_udp_send_t* send_req;
    send_req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
    status = uv_udp_send(send_req, &server, &read_buf, 1, (const struct sockaddr*)&send_addr, on_udp_send);
    CHECK(status,"send");
}

int main(int argc, const char * argv[]) {
   
    uv_loop = uv_default_loop();
    
    status = uv_udp_init(uv_loop,&server);
    CHECK(status,"init");
    
    status = uv_udp_init(uv_loop,&send_server);
    CHECK(status,"init");
    
    uv_ip4_addr("0.0.0.0", 11000, &addr);

    status = uv_udp_bind(&server, (const struct sockaddr*)&addr,0);
    CHECK(status,"bind");

    status = uv_udp_recv_start(&server, on_alloc, on_recv);
    CHECK(status,"recv");
    
    std::cout<<"server runing port 11000..."<< std::endl;
    uv_run(uv_loop, UV_RUN_DEFAULT);
    
    
    return 0;
}
