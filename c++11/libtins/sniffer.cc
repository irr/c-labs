// http://libtins.github.io
// g++ -std=c++0x -o sniffer -Wl,-static -static-libgcc sniffer.cc -L/usr/local/lib -lpthread -lrt -ltins -lpcap

#include <tins/tins.h>

#include <unistd.h>

#include <csignal>

#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <algorithm>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

static std::mutex MUTEX;

static const std::time_t EXPIRES = 60;

using namespace Tins;

class Stream {
    friend std::ostream &operator<<(std::ostream&, const Stream&);
    public:
        std::string id;
        bool ignore;
        std::time_t timestamp;

        uint32_t sent;
        uint32_t received;

        Stream() {
            this->id = "";
            this->ignore = true;
            this->timestamp = 0;
            this->sent = 0;
            this->received = 0;
        }

        virtual ~Stream() {}

        Stream(const Stream& st) {
            this->id = st.id;
            this->ignore = st.ignore;
            this->timestamp = st.timestamp;
            this->sent = st.sent;
            this->received = st.received;
        }

        Stream& operator=(const Stream &rhs) {
           this->id = rhs.id;
           this->ignore = rhs.ignore;
           this->timestamp = rhs.timestamp;
           this->sent = rhs.sent;
           this->received = rhs.received;
           return *this;
        }

        int operator==(const Stream& rhs) const {
            return (this->id == rhs.id);
        }

        int operator<(const Stream& rhs) const {
            return (this->id < rhs.id);
        }

        bool is_expired(const std::time_t& secs) const {
            return ((this->timestamp + secs) < std::time(nullptr));
        }
};

std::ostream& operator<<(std::ostream &output, const Stream& st) {
   output << st.id << ',' << st.sent << "," << st.received << "," << st.ignore << "," << st.timestamp << std::endl;
   return output;
}

std::map<const std::string, Stream> sessions;
std::vector<std::pair<std::string, Stream*>> tracker;

void gc() {
    tracker.erase(std::remove_if(tracker.begin(), 
                                 tracker.end(),
                                 [](std::pair<const std::string&, const Stream*> p) { 
                                       if (p.second->is_expired(EXPIRES)) {
                                           std::cout << ">>>>>>>>>>>>> EXPIRED! " << *p.second << std::endl;
                                           sessions.erase(sessions.find(p.first));
                                       }
                                       return p.second->is_expired(EXPIRES);
                                 }), tracker.end());
}

void signal_callback_handler(int signum) {
    std::lock_guard<std::mutex> guard(MUTEX);

    if (signum == SIGINT) {
        printf("Ctrl-C {signum=%d} detected. Exiting...\n", signum);
        exit(0);
    }

    gc();

    std::cout << "=======================================================" << std::endl;

    std::time_t now = std::time(nullptr);
    std::cout << "tracker contents @" << now << std::endl;

    for (const auto& elem : tracker) {
        std::cout << (*elem.second) << std::endl;
    }

    std::cout << "=======================================================" << std::endl;

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

bool ignore(const std::string& id) {
    std::lock_guard<std::mutex> guard(MUTEX);
    if (sessions.find(id) != sessions.end()) {
        const Stream st = sessions[id];
        if (st.ignore) {
            return true;
        }
    }
    return false;
}

bool stats(const TCPStream& tcp) { 
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

    if (ignore(id)) {
        std::cout << id << " (binary payload ignored)" << std::endl;
        return true;
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

    {
        std::lock_guard<std::mutex> guard(MUTEX);

        auto it = sessions.find(id);

        if (it == sessions.end()) {
            Stream st;
            st.id = id;
            st.ignore = ignore;
            st.timestamp = std::time(nullptr);
            st.sent = tcp.client_payload().size();
            st.received = tcp.server_payload().size();
            sessions[id] = st;
            tracker.push_back(std::make_pair(id, &sessions[id]));
            std::cout << ">>>>>>>>>>>>> ADDED! " << st << std::endl;
        } else {
            Stream& st = sessions[id];
            st.sent += tcp.client_payload().size();
            st.received += tcp.server_payload().size();
            if (!st.is_expired(EXPIRES)) {
                std::cout << ">>>>>>>>>>>>> TABLE! " << st << std::endl;
            }
        }

        gc();
    }

    return true;
}

void http_follower() {
    Sniffer sniffer("eth0");
    
    TCPStreamFollower stalker = TCPStreamFollower();
    stalker.follow_streams(sniffer, stats);
}

int main() {
    signal(SIGINT, signal_callback_handler);
    signal(SIGUSR1, signal_callback_handler);

    std::thread http{http_follower};
    http.join();

    return 1;
} 
