// http://libtins.github.io
// sudo yum install boost-devel glibc-static
// g++ -std=c++11 -o sniffer -Wl,-static -static-libgcc sniffer.cpp -L/usr/local/lib -lboost_iostreams -lrt -ltins -lpcap -lpthread

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

static const std::time_t EXPIRES = 60;

using namespace Tins;

class Stream {
    friend std::ostream &operator<<(std::ostream&, const Stream&);
    public:
        std::string id;
        std::time_t timestamp;

        std::string::size_type client_pos;
        std::string::size_type client_size;

        std::string::size_type server_pos;
        std::string::size_type server_size;

        Stream() {
            this->id = "";
            this->timestamp = 0;
            this->client_pos = 0;
            this->client_size = 0;
            this->server_pos = 0;
            this->server_size = 0;
        }

        virtual ~Stream() {}

        Stream(const Stream& st) {
            this->id = st.id;
            this->timestamp = st.timestamp;
            this->client_pos = st.client_pos;
            this->client_size = st.client_size;
            this->server_pos = st.server_pos;
            this->server_size = st.server_size;
        }

        Stream& operator=(const Stream &rhs) {
           this->id = rhs.id;
           this->timestamp = rhs.timestamp;
           this->client_pos = rhs.client_pos;
           this->client_size = rhs.client_size;
           this->server_pos = rhs.server_pos;
           this->server_size = rhs.server_size;
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

        bool is_expired(const std::time_t& secs) const {
            return ((this->timestamp + secs) < std::time(nullptr));
        }
};

std::ostream& operator<<(std::ostream &output, const Stream& st) {
   output << st.id << ',' << st.client_size << "," << st.server_size << "," << st.timestamp << std::endl;
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

void inspect(const std::string& id, const TCPCapStream& tcp, const std::string& s) noexcept {
    const auto& n = s.find("\r\n\r\n");
    if (n > 0) {
        std::cout << s.substr(0, n) << "...\n" << std::endl;
    } else {
        std::cout << "(HTTP payload error)" << std::endl;
    }
}

std::pair<const std::string, const std::string&> idlog(const TCPStream& tcp) {
    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string id = str(boost::format{"%1%:%2%|%3%:%4%"} 
                                % info.client_addr.to_string()
                                % info.client_port
                                % info.server_addr.to_string()
                                % info.server_port);

    const std::string lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%"}
                                % tcp.id()
                                % id
                                % client_payload.size()
                                % server_payload.size()
                                % tcp.is_finished());

    return std::make_pair(id, lg);
}

bool http_fin(const TCPCapStream& tcp) noexcept { 
    std::lock_guard<std::mutex> guard(MUTEX);

    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string id = str(boost::format{"%1%:%2%|%3%:%4%"} 
                                % info.client_addr.to_string()
                                % info.client_port
                                % info.server_addr.to_string()
                                % info.server_port);

    const std::string lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%"}
                                % tcp.id()
                                % id
                                % client_payload.size()
                                % server_payload.size()
                                % tcp.is_finished());

    std::cout << "FINISHED!: " << lg << std::endl;
}

bool http_cap(const TCPCapStream& tcp) noexcept { 
    std::lock_guard<std::mutex> guard(MUTEX);

    const RawPDU::payload_type& client_payload = tcp.client_payload();
    const RawPDU::payload_type& server_payload = tcp.server_payload();

    const TCPCapStream::StreamInfo& info = tcp.stream_info();

    const std::string id = str(boost::format{"%1%:%2%|%3%:%4%"} 
                                % info.client_addr.to_string()
                                % info.client_port
                                % info.server_addr.to_string()
                                % info.server_port);

    const std::string lg = str(boost::format{"http,0x%1$08x,%2%,%3%,%4%,%5%"}
                                % tcp.id()
                                % id
                                % client_payload.size()
                                % server_payload.size()
                                % tcp.is_finished());

    std::cout << "LOG: " << lg << std::endl;

    const std::string client_tcpstream(client_payload.begin(), client_payload.end());
    const std::string server_tcpstream(server_payload.begin(), server_payload.end());

    std::cout << "CLIENT PAYLOAD: " << client_tcpstream << std::endl;
    std::cout << "SERVER PAYLOAD: " << server_tcpstream << std::endl;

    return true;

    /*
    auto client_methods = { "GET", "POST" };
    auto client_pos = std::string::npos;

    for (const auto& method : client_methods) {
        client_pos = client_tcpstream.find("GET", st.client_pos);
        if (client_pos != std::string::npos) {
            client_pos = client_tcpstream.find("HTTP/1.", client_pos);
            if (client_pos != std::string::npos) {
                inspect(id, tcp, client_tcpstream);
                std::cout << lg << std::endl;
                break;
            }
        } 
    }
  
    auto server_pos = server_tcpstream.find("HTTP/1.", st.server_pos);
    if (server_pos != std::string::npos) {
       inspect(id, tcp, server_tcpstream);
       std::cout << lg << std::endl;
    } 

    {
        if (st.is_empty()) {
            st.id = id;
            st.timestamp = std::time(nullptr);
            st.client_pos = tcp.client_payload().size();
            st.client_size = st.client_pos;
            st.server_pos = tcp.server_payload().size();
            st.server_size = st.server_pos;
            sessions.emplace(std::make_pair(id, std::move(st)));
            tracker.push_back(std::make_pair(id, &sessions[id]));
            std::cout << ">>>>>>>>>>>>> ADDED! " << st << std::endl;
        } else {
            auto size = tcp.client_payload().size();
            if (size != st.client_size) {
                st.client_pos = ++client_pos;
                st.client_size = size;
            }
            size = tcp.server_payload().size();
            if (size != st.server_size) {
                st.server_pos = ++server_pos;
                st.server_size = size;
            }
            if (!st.is_expired(EXPIRES)) {
                std::cout << ">>>>>>>>>>>>> TABLE! " << st << std::endl;
            }
        }

        gc();
    }

    return true;
    */
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

