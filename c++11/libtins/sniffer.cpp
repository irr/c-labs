// http://libtins.github.io
// sudo yum install boost-devel glibc-static
// g++ -std=c++11 -o sniffer -Wl,-static -static-libgcc sniffer.cpp tcpcap_stream.cpp -L/usr/local/lib -lboost_iostreams -lrt -ltins -lpcap -lpthread

#include <tins/tins.h>

#include <unistd.h>

#include <csignal>

#include <cstddef>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <utility>

#include <boost/format.hpp>

#include "tcpcap_stream.h"

static std::mutex MUTEX;

static const std::time_t EXPIRES = 3600;

using namespace Tins;

class Stream {
    friend std::ostream &operator<<(std::ostream&, const Stream&);
    public:
        std::string id;
        std::time_t timestamp;

        std::string::size_type sent;
        std::string::size_type recv;

        Stream() {
            this->id = "";
            this->timestamp = 0;
            this->sent = 0;
            this->recv = 0;
        }

        virtual ~Stream() {}

        Stream(const Stream& st) {
            this->id = st.id;
            this->timestamp = st.timestamp;
            this->sent = st.sent;
            this->recv = st.recv;
        }

        Stream& operator=(const Stream& rhs) {
           this->id = rhs.id;
           this->timestamp = rhs.timestamp;
           this->sent = rhs.sent;
           this->recv = rhs.recv;
           return *this;
        }

        int operator==(const Stream& rhs) const {
            return (this->id == rhs.id);
        }

        int operator<(const Stream& rhs) const {
            return (this->id < rhs.id);
        }

        bool is_empty() const {
            return (this->id == "");
        }

        void sent_bytes(const std::string::size_type& sent) {
            this->sent += sent;
        }

        void recv_bytes(const std::string::size_type& recv) {
            this->recv += recv;
        }

        bool is_expired(const std::time_t& secs) const {
            return ((this->timestamp + secs) < std::time(nullptr));
        }
};

std::ostream& operator<<(std::ostream &output, const Stream& st) {
   output << st.id << ',' << st.sent << "," << st.recv << "," << st.timestamp << std::endl;
   return output;
}

std::map<const std::string, Stream> sessions;
std::vector<std::pair<std::string, Stream*>> tracker;

void gc() noexcept {
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

void signal_callback_handler(int signum) noexcept {
    std::lock_guard<std::mutex> guard(MUTEX);

    if (signum == SIGINT) {
        std::cout << boost::format("\nCtrl-C signal {signum=%1%}\n") % signum;
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
    std::cout << boost::format("Caught signal {signum=%1%}\n") % signum;
}

const std::string get_id(const TCPCapStream::StreamInfo& info) {
    return str(boost::format{"%1%:%2%|%3%:%4%"} 
                                % info.client_addr.to_string()
                                % info.client_port
                                % info.server_addr.to_string()
                                % info.server_port);
}

bool http_fin(const TCPCapStream& tcp) noexcept { 
    std::lock_guard<std::mutex> guard(MUTEX);

    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string& id = get_id(info);

    auto it = sessions.find(id); 

    const std::string& lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%"}
                                % tcp.id()
                                % id
                                % it->second.sent
                                % it->second.recv
                                % tcp.is_finished());

    std::cout << "FIN: " << lg << std::endl;
}

bool http_cap(const TCPCapStream& tcp) noexcept { 
    std::lock_guard<std::mutex> guard(MUTEX);

    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string& id = get_id(info);
    
    auto it = sessions.find(id); 
    if (it != sessions.end()) {
        if (!it->second.is_expired(EXPIRES)) {
            it->second.sent_bytes(client_payload.size());
            it->second.recv_bytes(server_payload.size());
            std::cout << ">>>>>>>>>>>>> TABLE! " << &(it->second) << std::endl;
        }
    } else {
        Stream st;
        st.id = id;
        st.timestamp = std::time(nullptr);
        st.sent = client_payload.size();
        st.recv = server_payload.size();
        sessions.emplace(std::move(std::make_pair(id, st)));
        tracker.push_back(std::make_pair(id, &sessions[id]));
        std::cout << ">>>>>>>>>>>>> ADDED! " << &sessions[id] << std::endl;
    }

    const std::string& lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%"}
                                % tcp.id()
                                % id
                                % client_payload.size()
                                % server_payload.size()
                                % tcp.is_finished());

    std::cout << "CAP: " << lg << std::endl;

    gc();

    return true;
}

void http_follower() noexcept {
    SnifferConfiguration config;
    config.set_filter("tcp and port 80");
    config.set_promisc_mode(true);

    Sniffer sniffer("eth0", config);
    TCPCapStreamFollower().follow_streams(sniffer, http_cap, http_fin);
}

int main() {
    signal(SIGINT, signal_callback_handler);
    signal(SIGUSR1, signal_callback_handler);

    std::vector<std::function<decltype(http_follower)>> funcs = { http_follower };
    
    std::vector<std::thread> threads;
    for (const auto& f : funcs) threads.push_back(std::thread(f));

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return 1;
} 

