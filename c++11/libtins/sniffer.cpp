// http://libtins.github.io
// sudo ./b2 threading=multi link=static --layout=tagged install --prefix=/usr/local
// g++ -std=c++11 -o sniffer -Wl,-static -static-libgcc sniffer.cpp tcpcap_stream.cpp -L/usr/local/lib -lboost_iostreams -lrt -ltins -lpcap -lpthread

#include <tins/tins.h>

#include <unistd.h>

#include <csignal>

#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <ctime>

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/regex.hpp>

#include "tcpcap_stream.hpp"

static std::mutex MUTEX;

static const std::time_t SECS_TO_EXPIRE = 3600;
static const double      SECS_TO_GC = 300;

enum class Flow : std::int8_t {UNKNOWN = 0, CLIENT = 1, SERVER = 2};

const std::string fmt_time_secs(const timespec& ts) noexcept  {
    const int n = 8;
    unsigned long long ns = ts.tv_sec * 1000000000 + ts.tv_nsec;
    long double secs = (ns / (long double) 1000000000.0);
    const std::string f = (boost::format("%%%d.%df") % (n << 1) % n).str();
    return str(boost::format(f) % secs);
}

unsigned long long diff_time_ns(const timespec& ns_start, const timespec& ns_end) noexcept {
    unsigned long long ns1 = ns_start.tv_sec * 1000000000 + ns_start.tv_nsec;
    unsigned long long ns2 = ns_end.tv_sec * 1000000000 + ns_end.tv_nsec;
    return (ns2 - ns1);
}

long double diff_time_ms(const timespec& ns_start, const timespec& ns_end) noexcept {
    return ((long double) diff_time_ns(ns_start, ns_end) / 1000000);
}

std::string decode64(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), 
            [](char c) {
                return c == '\0';
    });
}

std::string encode64(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

using namespace Tins;

class Stream {
    friend std::ostream &operator<<(std::ostream&, const Stream&);
    public:
        static std::vector<Stream> stats;
        std::string id;
        std::string data;
        timespec initial;
        timespec last;
        std::string::size_type sent;
        std::string::size_type recv;
        Flow state;

        Stream() {
            this->id = "";
            this->data = "";
            this->sent = 0;
            this->recv = 0;
            this->state = Flow::UNKNOWN;
            clock_gettime(CLOCK_REALTIME, &this->initial);
            clock_gettime(CLOCK_REALTIME, &this->last);
        }

        virtual ~Stream() {}

        Stream(const Stream& st) {
            this->id = st.id;
            this->data = st.data;
            this->initial = st.initial;
            this->last = st.last;
            this->sent = st.sent;
            this->recv = st.recv;
            this->state = st.state;
        }

        Stream& operator=(const Stream& rhs) {
           this->id = rhs.id;
           this->data = rhs.data;
           this->initial = rhs.initial;
           this->last = rhs.last;
           this->sent = rhs.sent;
           this->recv = rhs.recv;
           this->state = rhs.state;
           return *this;
        }

        int operator==(const Stream& rhs) const {
            return (this->id == rhs.id);
        }

        int operator<(const Stream& rhs) const {
            return (this->id < rhs.id);
        }

        void touch() {
            clock_gettime(CLOCK_REALTIME, &this->last);
        }

        void stat(const Flow& flow, const std::string& data) {
            Stream st;
            st.id = this->id;
            st.data = data;
            st.initial = this->initial;
            st.last = this->last;
            st.sent = this->sent;
            st.recv = this->recv;
            st.state = flow;
            stats.push_back(st);
            std::cout << st << std::endl;
            this->sent = 0;
            this->recv = 0;
            this->state = flow;
            clock_gettime(CLOCK_REALTIME, &this->initial);
            clock_gettime(CLOCK_REALTIME, &this->last);
        }

        void stat(const Flow& flow, const std::string::size_type& size) {
            Stream& st = stats.back();
            switch (flow) {
                case Flow::CLIENT: {
                    st.sent = st.sent + size;
                    st.last = this->last;
                    break;
                }
                case Flow::SERVER: {
                    st.recv = st.recv + size;
                    st.last = this->last;
                    break;
                }
            }
        }

        bool is_expired(const std::time_t& secs) const {
            timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            return ((this->initial.tv_sec + secs) < ts.tv_sec);
        }
};

std::vector<Stream> Stream::stats;

std::ostream& operator<<(std::ostream &output, const Stream& st) {
   static const std::vector<std::string> STATES = { "UNK", "CLI", "SRV" };
   output << st.id << ",[" << st.data << "]," << STATES.at(static_cast<std::int8_t>(st.state)) << "," 
          << st.sent << "," << st.recv << "," 
          << fmt_time_secs(st.initial) << "," << fmt_time_secs(st.last) << ":"
          << diff_time_ms(st.initial, st.last);
   return output;
}

std::map<const std::string, Stream> sessions;

void gc() {
    static std::time_t last = std::time(nullptr);
    std::time_t now = std::time(nullptr);

    if (std::difftime(now, last) > SECS_TO_GC) {
        for (auto it = sessions.begin(), ite = sessions.end(); it != ite;) {
            if (it->second.is_expired(SECS_TO_EXPIRE)) {
                std::cout << "STAT:" << it->second << std::endl;
                it = sessions.erase(it);
            }
            else {
                ++it;
            }
        }
        last = std::time(nullptr);
    }
}

void signal_callback_handler(int signum) {
    std::lock_guard<std::mutex> guard(MUTEX);

    if (signum == SIGINT) {
        std::cout << boost::format("\nCtrl-C signal {signum=%1%}\n") % signum;
        exit(0);
    }

    gc();

    std::vector<std::string> tab;
    std::string::size_type n = 0;

    std::for_each(sessions.begin(), sessions.end(), 
                  [&tab, &n](const std::pair<std::string, Stream>& elem) {
                        std::stringstream line;
                        line << elem.second;
                        const std::string s = line.str();
                        if (s.length() > n) n = s.length();
                        tab.push_back(s);
                    });

    std::for_each(Stream::stats.begin(), Stream::stats.end(), 
                  [&tab, &n](const Stream& st) {
                        std::stringstream line;
                        line << st;
                        const std::string s = line.str();
                        if (s.length() > n) n = s.length();
                        tab.push_back(s);
                    });

    if (n > 0) {
        const std::string f = (boost::format("| %%-%ds |\n") % n).str();
        n += 4;
        std::cout << std::string(n, '-') << std::endl;
        std::for_each(tab.begin(), tab.end(), 
                      [&f](const std::string& line) {
                           std::cout << boost::format(f) % line;
                        });
        std::cout << std::string(n, '-') << std::endl;
    }

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
        const std::string& lg = str(boost::format{"http,0x%1$08x,%2%"}
                                    % tcp.id()
                                    % id);
        std::cout << "FIN: " << lg << std::endl;
        sessions.erase(it);
    }
}

const std::string http_command(const std::string& pattern, const std::string& data) {
    const boost::regex expr{pattern};
    boost::smatch what;
    if (boost::regex_search(data, what, expr)) {
        return what[1];
    }
    return "";
}

bool http_inspect(Stream* pst, const Flow& flow, const std::string& payload, 
                  std::string mark, const std::string& lg){
    if (payload.length() > 0) {
        switch (pst->state) {
            case Flow::UNKNOWN: {
                std::size_t found = payload.find(mark);
                if (found != std::string::npos) {
                    std::cout << "CAP: " << lg << std::endl;
                    std::size_t limit = payload.find("\r\n\r\n", found);
                    if (limit != std::string::npos) {
                        const std::string& data = payload.substr(found, limit);
                        switch (flow) {
                            case Flow::CLIENT: {
                                const std::string uri = http_command(str(
                                            boost::format{"%1%\\s*?(\\S+).*?"} % mark), data);
                                const std::string host = http_command("^Host:\\s*?(\\S+).*?", data);
                                boost::trim(mark);
                                boost::property_tree::basic_ptree<std::string, std::string> parts;
                                parts.put<std::string>("method", mark);
                                parts.put<std::string>("uri", encode64(uri));
                                parts.put<std::string>("host", host);
                                boost::property_tree::ptree root;
                                root.put_child("json", parts);
                                std::stringstream ss;
                                boost::property_tree::json_parser::write_json(ss, root, false);
                                pst->stat(Flow::CLIENT, ss.str());
                                break;
                            }
                            case Flow::SERVER: {
                                std::string status = http_command(str(
                                            boost::format{"%1%\\d+\\.\\d+\\s*?(\\S+).*?"} % mark), data);
                                std::string version = http_command(str(
                                            boost::format{"%1%(\\d+\\.\\d+)\\s*?\\S+.*?"} % mark), data);
                                boost::property_tree::basic_ptree<std::string, std::string> parts;
                                parts.put<std::string>("status", status);
                                parts.put<std::string>("version", version);
                                boost::property_tree::ptree root;
                                root.put_child("json", parts);
                                std::stringstream ss;
                                boost::property_tree::json_parser::write_json(ss, root, false);
                                pst->stat(Flow::SERVER, ss.str());
                                break;
                            }
                        }
                        return true;
                    }
                }
                break;
            }
            case Flow::CLIENT: {
                pst->stat(Flow::CLIENT, payload.size());
                break;
            }
            case Flow::SERVER: {
                pst->stat(Flow::SERVER, payload.size());
                break;
            }
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
   
    Stream* pst;
    Stream st;

    auto it = sessions.find(id); 
    if (it != sessions.end()) {
        pst = &it->second;
        if (!it->second.is_expired(SECS_TO_EXPIRE)) {
            it->second.sent += client_payload.size();
            it->second.recv += server_payload.size();
            it->second.touch();
        }
    } else {
        st.id = id;
        st.sent = client_payload.size();
        st.recv = server_payload.size();
        sessions.emplace(std::make_pair(id, st));
        pst = &st;
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
        if (http_inspect(pst, Flow::CLIENT, client_tcpstream, method, lg)) {
            skip = true;
            break;
        }
    }
   
    if ((!skip) && (!http_inspect(pst, Flow::SERVER, server_tcpstream, "HTTP/", lg))) {
       std::cout << "CHK: " << lg << std::endl;
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
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = signal_callback_handler;

    sigaction(SIGINT,  &act, 0);
    sigaction(SIGUSR1, &act, 0);

    std::vector<std::function<decltype(http_follower)>> funcs = { http_follower };
    
    std::vector<std::thread> threads;
    for (const auto& f : funcs) threads.push_back(std::thread(f));

    std::cout << "sniffer started..." << std::endl;

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

    return 1;
} 

