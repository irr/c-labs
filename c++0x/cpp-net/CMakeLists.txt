CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(rclient CXX)
ADD_EXECUTABLE(rclient
rclient.cc)
INCLUDE_DIRECTORIES(/data/C++/cpp-netlib
/data/C/hiredis) 
TARGET_LINK_LIBRARIES(rclient
hiredis
boost_system
pthread)
