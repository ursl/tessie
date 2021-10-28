# tessie

Etymology: tessie sounds better than TC (box), for temperature cycling (box)


## hints for starting with test0:
git clone git@github.com:ursl/tessie

(git clone https://github.com/ursl/tessie.git)

cd tessie/test0/rpc

make mini

cd ../

qmake -o Makefile test0.pro

make


## RPC communication 
works from MAC and/or linux to raspberry pi (tessie.psi.ch)

cd rpc

make all

./trpc_client tessie settemp 17

./trpc_client tessie gettemp

