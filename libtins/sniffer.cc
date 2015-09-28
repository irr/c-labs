// http://libtins.github.io
// g++ -std=c++0x -o sniffer -Wl,-static -static-libgcc sniffer.cc -L/usr/local/lib -lpthread -lrt -ltins -lpcap

#include <tins/tins.h>

#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <cstdio>

#include <string>

using namespace Tins;
 
void signal_callback_handler(int signum) {
    if (signum == SIGINT) {
        printf("Ctrl-C {signum=%d} detected. Exiting...\n", signum);
        exit(0);
    }

    printf("Caught signal {signum=%d}\n", signum);
}

void inspect(const std::string& id, const size_t& hash, const TCPStream& tcp, const std::string& s) {
    const auto& n = s.find("\r\n\r\n");
    if (n > 0) {
        std::cout << s.substr(0, n) << "...\n" << std::endl;
    } else {
        std::cout << "(HTTP payload error)" << std::endl;
    }
}

bool stats(TCPStream tcp) { 
    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const auto& payload = (server_payload.size() > 0) ? 
                           server_payload : client_payload;

    const TCPStream::StreamInfo& info = tcp.stream_info();

    std::string id;
    id.append(info.client_addr.to_string());
    id.append(":");
    id.append(std::to_string(info.client_port));
    id.append("|");
    id.append(info.server_addr.to_string());
    id.append(":");
    id.append(std::to_string(info.server_port));

    std::hash<std::string> hash_fn;
    const size_t hash = hash_fn(id);

    printf("0x%08lx,%s{%s},%d,%d,%d,%d\n",
            tcp.id(), id.c_str(), std::to_string(hash).c_str(),
            client_payload.size(), server_payload.size(), payload.size(),
            tcp.is_finished());

    const std::string tcpstream(payload.begin(), payload.end());
    if (tcpstream.find("HTTP/1.") >= 0) {
        inspect(id, hash, tcp, tcpstream);
    } else {
        std::cout << "(unknown payload)" << std::endl;
    }

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
