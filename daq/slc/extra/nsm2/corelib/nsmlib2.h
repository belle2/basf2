/* -- INCLUDE FILES -------------------------------------------------- */
#include <netinet/in.h>  /* for struct sockaddr_in */

#include "nsm2.h"
#include "nsmsys2.h"

/* -- DATA TYPES ----------------------------------------------------- */
#if 0
#ifndef __nsm2_typedef__
#define __nsm2_typedef__
typedef unsigned char      byte8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef signed   short     int16;
typedef signed   int       int32;
typedef signed   long long int64;
#endif /* nsm2_typedef */

#ifndef __nsms2_typedef__
#define __nsms2_typedef__
typedef struct sockaddr    SOCKAD;
typedef struct sockaddr_in SOCKAD_IN;
#endif /* nsms2_typedef__ */
#endif

#ifndef __nsmc2_typedef__
#define __nsmc2_typedef__
struct NSMcontext_struct;
typedef struct NSMcontext_struct NSMcontext;
#endif /* nsmc2_typedef */

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

#if 0
typedef enum NSMreqtype_enum {
  NSMLIB_RQNON, /* no entry */
  NSMLIB_RQALL, /* any request which is not found */
  NSMLIB_RQSYS, /* nsmd request */
  NSMLIB_RQINV, /* invalid nsmd request */
  NSMLIB_RQMSG, /* standard message */
  NSMLIB_RQERR, /* error message */
  NSMLIB_RQREQ, /* standard request */
  NSMLIB_RQDAT, /* request with attached data */
} NSMreqtype;
#endif

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

#if 0
struct NSMcontext_struct {
  FILE*      logfp;
  NSMrequest req[NSMLIB_MAX_CALLBACK];
  int        nreq;
  NSMsys*    sysp;
  char*      memp;
  char       nodename[NSMSYS_NAME_SIZ + 1];
  char*      aptr;  /* default allocated ptr for obsolete use */
  char*      dptr;  /* default data pointer for obsolete use */
  int        nodeid;
  int        sock;
  int        seq;
  int        membase;
  int        reqwait;
  int        syncpipe_rd;  /* 0: for read, 1: for write */
  int        syncpipe_wr;  /* 0: for read, 1: for write */
  int        pars1;
  char       hostname[128];
  int        bufid;
  int        port;
  struct sockaddr_in sa;

  int        debug;
  int        nsmerrno;
  char*      defaultdest;
  int        npipedata;
  int        shmsysid;
  int        shmmemid;

  int        maxrecursive;
  int        currecursive;

  int        initnet_done;
  int        initshm_done;
  int        inithan_done;
  int        initcli_done;

  struct NSMcontext_struct* next;
};
#endif


const char* nsmlib_nodename(NSMcontext* nsmc, int nodeid);
const char* nsmlib_reqname(NSMcontext* nsmc, int reqid);
const char* nsmlib_strerror(NSMcontext*);
int nsmlib_callback(NSMcontext* nsmc, const char* name,
                    NSMcallback_t callback, NSMfunctype functype);
void* nsmlib_openmem(NSMcontext*, const char* dat, const char* fmt, int rev);
void* nsmlib_allocmem(NSMcontext*,
                      const char* dat, const char* fmt, int rev, float cycle);
int nsmlib_sendreq(NSMcontext*,
                   const char* node, const char* req, uint npar, int* pars, int len, const char* datap);
NSMcontext* nsmlib_init(const char*, const char*, int port, int shmkey);
void nsmlib_usesig(NSMcontext*, int usesig);
NSMcontext* nsmlib_selectc(int usesig, unsigned int msec);
void nsmlib_call(NSMcontext* nsmc, NSMtcphead* hp);
char* nsmlib_parse(const char* datname, int revision, const char* incpath);

/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
