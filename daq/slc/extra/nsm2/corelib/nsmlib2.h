/* -- INCLUDE FILES -------------------------------------------------- */
#include <netinet/in.h>  /* for struct sockaddr_in */

#include "nsm2.h"
#include "nsmsys2.h"

/* -- DATA TYPES ----------------------------------------------------- */
#ifndef __nsm2_typedef_context__
#define __nsm2_typedef_context__
struct NSMcontext_struct;
typedef struct NSMcontext_struct NSMcontext;
#endif /* nsm2_typedef_context */

#ifndef __nsm2_typedef_parse__
#define __nsm2_typedef_parse__
struct NSMparse_struct;
typedef struct NSMparse_struct NSMparse;
#endif /* nsm2_typedef_parse */

#define NSMSYS_NAME_SIZ 31

#define NSMLIB_MAX_RETRY 30
#define NSMLIB_WAIT_RETRY 2
#define NSMLIB_MAX_CALLBACK 1024

typedef int (*NSMhook_t)(NSMmsg* msg, NSMcontext* nsmc);

typedef enum NSMfunctype_enum {
  NSMLIB_FNNON, /* no entry */
  NSMLIB_FNSYS, /* for NSMfunc_t (NSM2 standard) with NSMmsg/from */
  NSMLIB_FNSTD, /* for NSMfunc_t (NSM2 standard) with NSMmsg/from */
  NSMLIB_FNREQ, /* for NSMfuncr_t with req/seq/pars/from */
  NSMLIB_FNDAT, /* for NSMfuncd_t with req/seq/pars/from/len/msg */
  NSMLIB_FNMSG, /* for NSMfuncm_t with req/seq/msg/from */
  NSMLIB_FNERR, /* for NSMfunce_t with req/seq/msg/from/node */
} NSMfunctype;

typedef struct {
  int req;
  NSMfunctype functype;
  NSMfunc_t callback;
  char name[64];
} NSMrequest;

typedef struct NSMrecvqueue_struct {
  struct NSMrecvqueue_struct* next;
  NSMtcphead h;
} NSMrecvqueue;

struct NSMcontext_struct {
  /* seq */
  int  seq;

  /* error code (when can't be returned) */
  int  errc;
  char errs[1024]; /* error string in case of NSMEUNEXPECTED */

  /* initnet (network related) */
  int  initnet_done;
  int  sock;
  int  port;
  int  shmkey;
  SOCKAD_IN sa;
  char nodename[NSMSYS_NAME_SIZ + 1];
  char hostname[1024];

  /* initshm (shared memory related) */
  int  initshm_done;
  int  sysid;
  NSMsys* sysp;
  int  memid;
  NSMmem* memp;

  /* initsig (signal handler related) */
  int  usesig;
  int  initsig_done;
  int  pipe_rd;
  int  pipe_wr;
  int  reqwait;
  int  maxrecursive;
  int  currecursive;
  int  nreq;
  NSMrequest req[NSMLIB_MAX_CALLBACK];

  /* initcli (client related) */
  int  nodeid;
  struct NSMcontext_struct* next;

  /* recv queue */
  NSMrecvqueue* recvqueue;

  /* callback hook */
  NSMhook_t hook;
};

const char* nsmlib_nodename(NSMcontext* nsmc, int nodeid);
const char* nsmlib_reqname(NSMcontext* nsmc, int reqid);
const char* nsmlib_strerror(NSMcontext* nsmc);
int nsmlib_callback(NSMcontext* nsmc, const char* name,
                    NSMcallback_t callback, NSMfunctype functype);
void* nsmlib_openmem(NSMcontext* nsmc,
                     const char* dat, const char* fmt, int rev);
void* nsmlib_allocmem(NSMcontext* nsmc,
                      const char* dat, const char* fmt, int rev, float cycle);
int nsmlib_sendreq(NSMcontext* nsmc,
                   const char* node, const char* req,
                   uint npar, int* pars, int len, const char* datap);
NSMcontext* nsmlib_init(const char* nodename, const char* host,
                        int port, int shmkey);
void nsmlib_usesig(NSMcontext* nsmc, int usesig);
NSMcontext* nsmlib_selectc(int usesig, unsigned int msec);
void nsmlib_call(NSMcontext* nsmc, NSMtcphead* hp);
//char *nsmlib_parsefile(const char *datname, int revision, const char *incpath,
//           char *fmtstr);

/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
