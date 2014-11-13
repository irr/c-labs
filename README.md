c-labs
------------

**c-labs**  is a set of sample codes whose main purpose is to experiment and test *c/c++11*

```shell
mkdir -p ~/.vim/colors
cp ~/git/c-labs/vim/calmar256-dark.vim ~/.vim/colors/
```

```shell
yum groupinstall "Development Tools"
yum install scons cmake kernel-devel openssl-devel boost-devel boost-doc \
            libevent-devel libevent-doc hiredis-devel readline-devel libffi-devel \
            libstdc++-devel glibc-devel glibc-static boost-static libstdc++-static \
            libmicrohttpd-devel libmicrohttpd-doc mariadb-devel sqlite-devel \
            valgrind
```

```shell 
apt-get install build-essential libboost-all-dev scons scons-doc cmake cmake-doc \
                c-cpp-reference stl-manual glibc-doc valgrind \
                witty witty-dbg witty-dev witty-doc libevent-dev libffi-dev \
                libmicrohttpd-devlibreadline6-dev libpcre3-dev libssl-dev \
                libsqlite3-dev libmysqlclient-dev \
                libncurses5-dev libelf-dev asciidoc binutils-dev c++-annotations

Copyright and License
-----------

Copyright 2012 Ivan Ribeiro Rocha

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
