#include "rpcServer.hh"
#include <iostream>
#include <unistd.h>
#include <sstream>

#include <chrono>
#include <thread>

#include "driveHardware.hh"

#include "rpc/trpc.h"

using namespace std;

float trpc_gtemperature;
driveHardware *trpc_ghw;
rpcServer *trpc_grpc;

// ----------------------------------------------------------------------
rpcServer::rpcServer(driveHardware *h) {
  trpc_ghw = h;
  trpc_grpc = this;

  // cout << "rpcServer::rpcServer() starting RPC server" << endl;
  // startServer();
}


// ----------------------------------------------------------------------
rpcServer::~rpcServer() {

}


// ----------------------------------------------------------------------
void rpcServer::sentToServer(const QString &result) {
  cout << "rpcServer::sentToServer() received ->" << result.toStdString() << "<-" << endl;
  emit("rpcServer result emitted");
}


// ----------------------------------------------------------------------
void rpcServer::run() {
  cout << "rpcServer::run() starting RPC server" << endl;
  startServer();
  cout << "rpcServer::run() after starting RPC server" << endl;

}

// ----------------------------------------------------------------------
void rpcServer::printFromServer(const QString &result) {
  emit sendFromServer(result);
}

// ----------------------------------------------------------------------
void *settemp_6_svc(args *argp, struct svc_req *rqstp) {
  static char * result;
  float temp;
  temp = argp->value;
  printf("now in settemp_6_svc, T = %f\n", temp);
  string sresult("now in settemp_6_svc, T = ");
  sresult += std::to_string(temp);
  QString qresult(QString::fromStdString(sresult));
  trpc_grpc->printFromServer(qresult);
  trpc_gtemperature = temp;
  /*
   * insert server code here
   */

  return (void *) &result;
}

// ----------------------------------------------------------------------
float *gettemp_6_svc(void *argp, struct svc_req *rqstp) {
  static float  result;

  printf("now in gettemp_6_svc, sending back T = %f\n", trpc_gtemperature);


  string sresult("now in gettemp_6_svc, sending back T = ");
  sresult += std::to_string(trpc_gtemperature);
  QString qresult(QString::fromStdString(sresult));
  trpc_grpc->printFromServer(qresult);

  /*
   * insert server code here
   */

  return &trpc_gtemperature;
}


/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

// ----------------------------------------------------------------------
void *settemp_6(args *argp, CLIENT *clnt) {
  static char clnt_res;

  memset((char *)&clnt_res, 0, sizeof(clnt_res));
  if (clnt_call (clnt, SETTEMP,
		 (xdrproc_t) xdr_args, (caddr_t) argp,
		 (xdrproc_t) xdr_void, (caddr_t) &clnt_res,
		 TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return ((void *)&clnt_res);
}

// ----------------------------------------------------------------------
float *gettemp_6(void *argp, CLIENT *clnt) {
  static float clnt_res;

  memset((char *)&clnt_res, 0, sizeof(clnt_res));
  if (clnt_call (clnt, GETTEMP,
		 (xdrproc_t) xdr_void, (caddr_t) argp,
		 (xdrproc_t) xdr_float, (caddr_t) &clnt_res,
		 TIMEOUT) != RPC_SUCCESS) {
    return (NULL);
  }
  return (&clnt_res);
}
