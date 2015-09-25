// http://libtins.github.io
// g++ -std=c++0x -o sniffer -Wl,-static -static-libgcc sniffer.cc -L/usr/local/lib -lpthread -lrt -ltins -lpcap

#include <tins/tins.h>

#include <tins/sniffer.h>
#include <tins/tcp.h>

#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <cstdio>

#include <string>

using namespace Tins;
 
std::ofstream outfile;

void signal_callback_handler(int signum) {
    printf("Caught signal %02d\n", signum);

    if (signum == SIGINT) {
        printf("Ctrl-C {%02d} detected. Exiting...\n", signum);
        exit(0);
    }
}

bool stats(TCPStream tcp) { 
    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const RawPDU::payload_type& payload = (server_payload.size() > 0) ? 
                                          server_payload : client_payload;

    TCPStream::StreamInfo info = tcp.stream_info();
    printf("0x%08lx,%s:%d,%s:%d,%d,%d,%d\n",
            tcp.id(), 
            info.client_addr.to_string().c_str(), info.client_port,
            info.server_addr.to_string().c_str(), info.server_port,
            client_payload.size(), server_payload.size(),
            tcp.is_finished());

    /*
    std::string tcpstream(payload.begin(),payload.end());
    std::cout<<tcpstream.substr(0, 40)<<std::endl;
    */

    return true;
}
 
int main() {
    signal(SIGINT, signal_callback_handler);
    signal(SIGUSR1, signal_callback_handler);

    Sniffer sniffer("eth0");
    
    TCPStreamFollower stalker = TCPStreamFollower();
    stalker.follow_streams(sniffer, stats);
    
    return 1;
} 
