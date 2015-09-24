// http://libtins.github.io

#include <tins/tins.h>

#include <tins/sniffer.h>
#include <tins/tcp.h>

#include <string>
#include <stdint.h>

using namespace Tins;
 
std::ofstream outfile;
 
bool stats(TCPStream tcp) { 
    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const RawPDU::payload_type& payload = (server_payload.size() > 0) ? 
                                          server_payload : client_payload;

    TCPStream::StreamInfo info = tcp.stream_info();
    std::cout << tcp.id() << "," 
              << info.client_addr << ":" << info.client_port << "<->" 
              << info.server_addr << ":" << info.server_port << ","
              << client_payload.size() << ","
              << server_payload.size() << ","
              << tcp.is_finished() << std::endl;

    /*
    std::string tcpstream(payload.begin(),payload.end());
    std::cout<<tcpstream.substr(0, 40)<<std::endl;
    */

    return true;
}
 
int main() {
    Sniffer sniffer("eth0");
    TCPStreamFollower stalker = TCPStreamFollower();
    stalker.follow_streams(sniffer, stats);
    return 1;
} 
