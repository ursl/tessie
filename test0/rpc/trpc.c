#include "trpc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rpc/rpc.h>


void pmap_unset(u_long prognum, u_long versnum);

// ----------------------------------------------------------------------
bool_t xdr_args (XDR *xdrs, args *objp) {
  register int32_t *buf;

  if (!xdr_float (xdrs, &objp->value))
    return FALSE;
  if (!xdr_char (xdrs, &objp->operation))
    return FALSE;
  return TRUE;
}

// ----------------------------------------------------------------------
void client_trpc_6(char *host, char *cmd, float argtemp) {
  CLIENT *clnt;
  void  *result_1;
  args  settemp_6_arg;
  float  *result_2;
  char *gettemp_6_arg;

#ifndef DEBUG
  clnt = clnt_create (host, TRPC, TRPC_VERS, "udp");
  printf("Hallo in DEBUG\n");
  if (clnt == NULL) {
    clnt_pcreateerror (host);
    exit (1);
  }
#endif  /* DEBUG */

  if (!strcmp("settemp", cmd)) {
    settemp_6_arg.value = argtemp;
    printf("calling from trpc_6 with settemp_6_arg.value = %f\n", settemp_6_arg.value);
    result_1 = settemp_6(&settemp_6_arg, clnt);
    if (result_1 == (void *) NULL) {
      clnt_perror (clnt, "call failed");
    }
  }
  if (!strcmp("gettemp", cmd)) {
    result_2 = gettemp_6((void*)&gettemp_6_arg, clnt);
    printf("calling from trpc_6, returned temperature = %f\n", *result_2);
    if (result_2 == (float *) NULL) {
      clnt_perror (clnt, "call failed");
    }
  }
#ifndef DEBUG
  clnt_destroy (clnt);
#endif   /* DEBUG */
}


// ----------------------------------------------------------------------
static void server_trpc_6(struct svc_req *rqstp, register SVCXPRT *transp) {
  union {
    args settemp_6_arg;
  } argument;
  char *result;
  xdrproc_t _xdr_argument, _xdr_result;
  char *(*local)(char *, struct svc_req *);

  switch (rqstp->rq_proc) {
  case NULLPROC:
    (void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
    return;

  case SETTEMP:
    _xdr_argument = (xdrproc_t) xdr_args;
    _xdr_result = (xdrproc_t) xdr_void;
    local = (char *(*)(char *, struct svc_req *)) settemp_6_svc;
    break;

  case GETTEMP:
    _xdr_argument = (xdrproc_t) xdr_void;
    _xdr_result = (xdrproc_t) xdr_float;
    local = (char *(*)(char *, struct svc_req *)) gettemp_6_svc;
    break;

  default:
    svcerr_noproc (transp);
    return;
  }
  memset ((char *)&argument, 0, sizeof (argument));
  if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
    svcerr_decode (transp);
    return;
  }
  result = (*local)((char *)&argument, rqstp);
  if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
    svcerr_systemerr (transp);
  }
  if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
    fprintf (stderr, "%s", "unable to free arguments");
    exit (1);
  }
  return;
}

// ----------------------------------------------------------------------
void startServer() {
  register SVCXPRT *transp;

  pmap_unset(TRPC, TRPC_VERS);

  transp = svcudp_create(RPC_ANYSOCK);
  if (transp == NULL) {
    fprintf (stderr, "%s", "cannot create udp service.");
    exit(1);
  }
  if (!svc_register(transp, TRPC, TRPC_VERS, server_trpc_6, IPPROTO_UDP)) {
    fprintf (stderr, "%s", "unable to register (TRPC, TRPC_VERS, udp).");
    exit(1);
  }

  transp = svctcp_create(RPC_ANYSOCK, 0, 0);
  if (transp == NULL) {
    fprintf (stderr, "%s", "cannot create tcp service.");
    exit(1);
  }
  if (!svc_register(transp, TRPC, TRPC_VERS, server_trpc_6, IPPROTO_TCP)) {
    fprintf (stderr, "%s", "unable to register (TRPC, TRPC_VERS, tcp).");
    exit(1);
  }

  svc_run ();
  fprintf (stderr, "%s", "svc_run returned");
  exit (1);
  /* NOTREACHED */
}
