# -- note that the comparison is LITERAL!
ifeq ($(OSTYPE),darwin)
  DIST := ../tessie.app/Contents/MacOS/rpc
  SOFLAGS += $(CFLAGS) -Wl -dynamiclib -shared /usr/lib/librpcsvc.dylib
else
  DIST = ../
  SOFLAGS +=
endif

CLIENT = trpc_client
SERVER = trpc_server

SOURCES_CLNT.c =
SOURCES_CLNT.h =
SOURCES_SVC.c =
SOURCES_SVC.h =
SOURCES.x = trpc.x

TARGETS_SVC.c = trpc_svc.c trpc_server.c trpc_xdr.c
TARGETS_CLNT.c = trpc_clnt.c trpc_client.c trpc_xdr.c
TARGETS = trpc.h trpc_xdr.c trpc_clnt.c trpc_svc.c trpc_client.c trpc_server.c

OBJECTS_CLNT = $(SOURCES_CLNT.c:%.c=%.o) $(TARGETS_CLNT.c:%.c=%.o)
OBJECTS_SVC = $(SOURCES_SVC.c:%.c=%.o) $(TARGETS_SVC.c:%.c=%.o)
# Compiler flags

CFLAGS += -g -O0 -W -Wall -fPIC -pipe
#LDLIBS += -lnsl
LDFLAGS += $(CFLAGS)
RPCGENFLAGS =

# -- main target creating the lib with the RPC source code
lib: trpc_clnt.o trpc_server.o trpc_server.o trpc.o
	$(CC)  $(SOFLAGS) trpc_clnt.o trpc_server.o trpc.o -o libtrpc.so
	echo "DIST = " $(DIST)
	mkdir -p $(DIST)
	cp libtrpc.so $(DIST)
# -- standalone original setup with server and client and RPC calls distributed in the respective source code
test : $(CLIENT) $(SERVER)

# -- 'main' server using RPC source code from libtrpc.so
tgui: tgui.o
	$(CXX)  $(CXXFLAGS) -c -o tgui.o tgui.cc
	$(CXX)  -o tgui tgui.o libtrpc.so


# -- all
all: clean lib tgui test

$(TARGETS) : $(SOURCES.x)
	rpcgen $(RPCGENFLAGS) $(SOURCES.x)

$(OBJECTS_CLNT) : $(SOURCES_CLNT.c) $(SOURCES_CLNT.h) $(TARGETS_CLNT.c)

$(OBJECTS_SVC) : $(SOURCES_SVC.c) $(SOURCES_SVC.h) $(TARGETS_SVC.c)

$(CLIENT) : $(OBJECTS_CLNT)
	$(LINK.c) -o $(CLIENT) $(OBJECTS_CLNT) $(LDLIBS)

$(SERVER) : $(OBJECTS_SVC)
	$(LINK.c) -o $(SERVER) $(OBJECTS_SVC) $(LDLIBS)

 clean:
	 $(RM) *.o *~ trpc_client trpc_server libtrpc.so tgui
