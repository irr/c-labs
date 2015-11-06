sudo yum install cmake glibc-static

devtoolset-2
============
http://people.centos.org/tru/devtools-2/readme
sudo wget http://people.centos.org/tru/devtools-2/devtools-2.repo -O /etc/yum.repos.d/devtools-2.repo
sudo yum install devtoolset-2-gcc devtoolset-2-binutils devtoolset-2-gcc-c++ devtoolset-2-elfutils devtoolset-2-valgrind devtoolset-2-dyninst devtoolset-2-strace.x86_64 devtoolset-2-gdb glibc-static libstdc++-static


libpcap
=======
cd /opt/c/libpcap-1.7.4
./configure
sudo make install


boost
=====
http://www.boost.org/more/getting_started/unix-variants.html
http://www.boost.org/build/doc/html/index.html

cd /opt/c/boost_1_59_0
sudo ./b2 threading=multi link=static --layout=tagged install --prefix=/usr/local
sudo ./b2 threading=multi --layout=tagged install --prefix=/usr/local


libtins
=======
cd /opt/c/libtins-3.2
mkdir build; cd build
cmake ../ -DLIBTINS_BUILD_SHARED=0 -DLIBTINS_ENABLE_CXX11=1 -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_DOT11=0 && make 
sudo make install
cmake ../ -DLIBTINS_BUILD_SHARED=1 -DLIBTINS_ENABLE_CXX11=1 -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_DOT11=0 && make 
sudo make install

