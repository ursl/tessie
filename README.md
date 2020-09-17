# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)


# hints for test0: 
git clone git@github.com:ursl/tessie

cd tessie/test0/rpc

make mini

cd ../

qmake -o Makefile test0.pro

make


# RPC communication now works from MAC to raspberry pi
but compiling the lib for the client on pi currently not finished
