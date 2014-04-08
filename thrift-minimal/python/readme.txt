[irocha@luma ~]$ pip freeze|grep thrift
thrift==0.8.0
If found (/usr/local/lib/python2.7/dist-packages/), remove thrift's version with:
sudo rm -rf /usr/lib/python2.7/site-packages/thrift*
