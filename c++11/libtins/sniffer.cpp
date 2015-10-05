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

#include "tcpcap_stream.hpp"

static std::mutex MUTEX;

static const std::time_t EXPIRES = 60;

using namespace Tins;

class Stream {
    friend std::ostream &operator<<(std::ostream&, const Stream&);
    public:
        std::string id;
        timespec timestamp;
        std::string::size_type sent;
        std::string::size_type recv;

        Stream() {
            this->id = "";
            this->sent = 0;
            this->recv = 0;
            clock_gettime(CLOCK_REALTIME, &this->timestamp);
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

        bool is_expired(const std::time_t& secs) const {
            timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            return ((this->timestamp.tv_sec + secs) < ts.tv_sec);
        }
};

const std::string fmt_time_secs(const timespec& ts) {
    unsigned long long ns = ts.tv_sec * 1000000000 + ts.tv_nsec;
    long double secs = (ns / (long double) 1000000000.0);
    const std::string f = (boost::format("%%%d.%df") % 16 % 8).str();
    return str(boost::format(f) % secs);
}

unsigned long long diff_time_ns(const timespec& ns_start, const timespec& ns_end) {
    unsigned long long ns1 = ns_start.tv_sec * 1000000000 + ns_start.tv_nsec;
    unsigned long long ns2 = ns_end.tv_sec * 1000000000 + ns_end.tv_nsec;
    return (ns2 - ns1);
}

std::ostream& operator<<(std::ostream &output, const Stream& st) {
   output << st.id << ',' << st.sent << "," << st.recv << "," << fmt_time_secs(st.timestamp) << std::endl;
   return output;
}

std::map<const std::string, Stream> sessions;
std::vector<std::pair<std::string, Stream*>> tracker;

void gc() {
    tracker.erase(std::remove_if(tracker.begin(), 
                                 tracker.end(),
                                 [](std::pair<const std::string&, const Stream*> p) { 
                                       if (p.second->is_expired(EXPIRES)) {
                                           std::cout << "EXPIRED! " << *p.second << std::endl;
                                           sessions.erase(sessions.find(p.first));
                                       }
                                       return p.second->is_expired(EXPIRES);
                                 }), tracker.end());
}

void signal_callback_handler(int signum) {
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

bool http_fin(const TCPCapStream& tcp) { 
    std::lock_guard<std::mutex> guard(MUTEX);

    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string& id = get_id(info);

    auto it = sessions.find(id); 

    if (it != sessions.end()) {
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        const std::string& lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%,%6%,%7%,%8%"}
                                    % tcp.id()
                                    % id
                                    % it->second.sent
                                    % it->second.recv
                                    % tcp.is_finished()
                                    % fmt_time_secs(it->second.timestamp).c_str()
                                    % fmt_time_secs(ts).c_str()
                                    % diff_time_ns(it->second.timestamp, ts));
        std::cout << "FIN: " << lg << std::endl;
    }
}

bool http_inspect(const std::string& payload, const std::string& mark, const std::string& lg){
    if (payload.length() > 0) {
        std::size_t found = payload.find(mark);
        if (found != std::string::npos) {
            std::cout << "CAP: " << lg << std::endl;
            std::size_t limit = payload.find("\r\n\r\n", found);
            if (limit != std::string::npos) {
                std::cout << payload.substr(found, limit) << std::endl;
            }
            return true;
        }
    }
    return false;
}

bool http_cap(const TCPCapStream& tcp) { 
    std::lock_guard<std::mutex> guard(MUTEX);

    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string& id = get_id(info);
    
    auto it = sessions.find(id); 
    if (it != sessions.end()) {
        if (!it->second.is_expired(EXPIRES)) {
            it->second.sent += client_payload.size();
            it->second.recv += server_payload.size();
        }
    } else {
        Stream st;
        st.id = id;
        st.sent = client_payload.size();
        st.recv = server_payload.size();
        sessions.emplace(std::make_pair(id, st));
        tracker.push_back(std::make_pair(id, &sessions[id]));
    }

    const std::string& lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%"}
                                % tcp.id()
                                % id
                                % client_payload.size()
                                % server_payload.size()
                                % tcp.is_finished());

    const std::string client_tcpstream(client_payload.begin(), client_payload.end());
    const std::string server_tcpstream(server_payload.begin(), server_payload.end());

    bool skip = false;

    std::vector<std::string> methods = { "GET ", "POST ", "PUT ", "DELETE ", "HEAD ", "OPTIONS " };
    for (const auto& method : methods) {
        if (http_inspect(client_tcpstream, method, lg)) {
            skip = true;
            break;
        }
    }
   
    if ((!skip) && (!http_inspect(server_tcpstream, "HTTP/", lg))) {
       std::cout << "BIN: " << lg << std::endl;
    }

    gc();

    return true;
}

void http_follower() {
    SnifferConfiguration config;
    config.set_filter("tcp and port 80");
    config.set_promisc_mode(true);

    Sniffer sniffer("any", config);
    TCPCapStreamFollower().follow_streams(sniffer, http_cap, http_fin);
}

int main() {
    signal(SIGINT, signal_callback_handler);
    signal(SIGUSR1, signal_callback_handler);

    std::vector<std::function<decltype(http_follower)>> funcs = { http_follower };
    
    std::vector<std::thread> threads;
    for (const auto& f : funcs) threads.push_back(std::thread(f));

    std::cout << "sniffer started..." << std::endl;

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return 1;
} 

