/* ---------------------------------------------------------------------- *\
   nsmlib2.h

   revision history can be found in nsmlib2.c
\* ---------------------------------------------------------------------- */

#ifndef __nsmlib2_h__
#define __nsmlib2_h__

/* -- INCLUDE FILES -------------------------------------------------- */
#include <stdio.h>       /* for FILE * */
#include <netinet/in.h>  /* for struct sockaddr_in */

#include "nsm2.h"

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

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

struct NSMtcphead_struct;
struct NSMrecvqueue_struct;
struct NSMmem_struct;
struct NSMsys_struct;

struct NSMcontext_struct {
  /* seq */
  int  seq;

  /* error code (when can't be returned) */
  int  errc;
  char errs[1024]; /* error string in case of NSMEUNEXPECTED */
  int  errn; /* errno in case of system error */

  /* initnet (network related) */
  int  initnet_done;
  int  sock;
  int  port;
  int  shmkey;
  struct sockaddr_in sa;
  char nodename[NSMSYS_NAME_SIZ + 1];
  char hostname[1024];

  /* initshm (shared memory related) */
  int  initshm_done;
  int  sysid;
  struct NSMsys_struct* sysp;
  int  memid;
  struct NSMmem_struct* memp;

  /* initsig (signal handler related) */
  int  usesig;
  int  initsig_done;
  int  pipe_rd;
  int  pipe_wr;
  int  reqwait;
  int  nreq;
  NSMrequest req[NSMLIB_MAX_CALLBACK];

  /* initcli (client related) */
  int  nodeid;
  struct NSMcontext_struct* next;

  /* recv queue */
  struct NSMrecvqueue_struct* recvqueue;

  /* callback hook */
  NSMhook_t hook;
  const void* hookptr;

  /* more pointer to be used by a wrapper */
  const void* wrapptr;
};


void nsmlib_logflush();
FILE* nsmlib_logging(FILE* logfp);
void nsmlib_checkpoint(NSMcontext* nsmc, int val);
int nsmlib_debuglevel(int val);
int nsmlib_addincpath(const char* path);
const char* nsmlib_nodename(NSMcontext* nsmc, int nodeid);
int nsmlib_nodeid(NSMcontext* nsmc, const char* nodename);
int nsmlib_nodepid(NSMcontext* nsmc, const char* nodename);
int nsmlib_nodeproc(NSMcontext* nsmc, const char* nodename);
int nsmlib_recv(NSMcontext* nsmc, struct NSMtcphead_struct* hp, int wait_msec);
int nsmlib_reqid(NSMcontext* nsmc, const char* reqname);
const char* nsmlib_reqname(NSMcontext* nsmc, int reqid);
const char* nsmlib_strerror(NSMcontext* nsmc);
int nsmlib_callback(NSMcontext* nsmc, const char* name,
                    NSMcallback_t callback, NSMfunctype functype);
int nsmlib_readmem(NSMcontext* nsmc, void* buf,
                   const char* dat, const char* fmt, int rev);
int nsmlib_statmem(NSMcontext* nsmc,
                   const char* dat, char* fmtbuf, int bufsiz);
void* nsmlib_openmem(NSMcontext* nsmc,
                     const char* dat, const char* fmt, int rev);
void* nsmlib_allocmem(NSMcontext* nsmc,
                      const char* dat, const char* fmt, int rev, float cycle);
int nsmlib_flushmem(NSMcontext* nsmc,
                    const void* ptr, int siz);
int nsmlib_sendreq(NSMcontext* nsmc,
                   const char* node, const char* req,
                   uint npar, int* pars, int len, const char* datap);
int nsmlib_register_request(NSMcontext* nsmc, const char* name);
NSMcontext* nsmlib_init(const char* nodename, const char* host,
                        int port, int shmkey);
int nsmlib_term(NSMcontext* nsmc);
void nsmlib_usesig(NSMcontext* nsmc, int usesig);
NSMcontext* nsmlib_selectc(int usesig, unsigned int msec);
void nsmlib_call(NSMcontext* nsmc, struct NSMtcphead_struct* hp);
NSMparse* nsmlib_parsefile(const char* datname, int revision,
                           const char* incpath, char* fmtstr, int* revisionp);

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif

#endif /* __nsmlib2_h__ */

/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
