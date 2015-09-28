// http://libtins.github.io
// g++ -std=c++0x -o sniffer -Wl,-static -static-libgcc sniffer.cc -L/usr/local/lib -lpthread -lrt -ltins -lpcap

#include <tins/tins.h>

#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <map>
#include <string>

using namespace Tins;

class Stream {
    friend std::ostream &operator<<(std::ostream&, const Stream&);
    public:
        std::string id;
        bool ignore;

        Stream() {}

        Stream(const Stream& st) {
            id = st.id;
            ignore = st.ignore;
        }

        ~Stream() {}
       
        Stream& operator=(const Stream &rhs) {
           this->id = rhs.id;
           this->ignore = rhs.ignore;
           return *this;
        }

        int operator==(const Stream& rhs) const {
            return (this->id == rhs.id);
        }

        int operator<(const Stream& rhs) const {
            return (this->id < rhs.id);
        }
};

std::ostream& operator<<(std::ostream &output, const Stream& st) {
   output << st.id << ' ' << std::to_string(st.ignore) << std::endl;
   return output;
}

std::map<const std::string, Stream> table;

void signal_callback_handler(int signum) {
    if (signum == SIGINT) {
        printf("Ctrl-C {signum=%d} detected. Exiting...\n", signum);
        exit(0);
    }

    printf("Caught signal {signum=%d}\n", signum);
}

void inspect(const std::string& id, const TCPStream& tcp, const std::string& s) {
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


    if(table.find(id) != table.end()) {
        const Stream st = table[id];
        if (st.ignore) {
            return true;
        }
    }

    printf("0x%08lx,%s,%d,%d,%d,%d\n",
            tcp.id(), id.c_str(), 
            client_payload.size(), server_payload.size(), payload.size(),
            tcp.is_finished());


    bool ignore = false;
    const std::string tcpstream(payload.begin(), payload.end());
    if ((tcpstream.find("HTTP/1.") == 0) ||  
        ((tcpstream.find("GET") == 0) && (tcpstream.find("HTTP/1.") >= 0)) ||  
        ((tcpstream.find("POST") == 0) && (tcpstream.find("HTTP/1.") >= 0))) {
        inspect(id, tcp, tcpstream);
    } else {
        std::cout << "(binary payload)" << std::endl;
        ignore = true;
    }

    if(table.find(id) == table.end()) {
        Stream st;
        st.id = id;
        st.ignore = ignore;
        table[id] = st;
        std::cout << ">>>>>>>>>>>>> ADDED! " << st << std::endl;
    } else {
        Stream st = table[id];
        std::cout << ">>>>>>>>>>>>> TABLE! " << st << std::endl;
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
