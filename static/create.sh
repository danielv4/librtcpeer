#
apt-get install -y build-essential

#
wget https://github.com/llamerada-jp/libwebrtc/releases/download/m78/libwebrtc-78.0.3904.108-ubuntu-18.04-x64.tar.gz

#
tar -xf libwebrtc-78.0.3904.108-ubuntu-18.04-x64.tar.gz

#
gcc -std=c99 -fPIC -c -o lfqueue.c.o lfqueue.c
g++ -I include -I include/third_party/abseil-cpp -I include/third_party/jsoncpp/source/include -I include/third_party/jsoncpp/generated -DWEBRTC_LINUX=1 -DWEBRTC_POSIX=1 -std=gnu++14 -fno-rtti librtcpeer.cpp lfqueue.c.o -fPIC -shared -o librtcpeer.so -L lib -lwebrtc -ldl -lpthread

#
rm /usr/lib/librtcpeer.so

#
cp librtcpeer.so /usr/lib/librtcpeer.so
ldconfig