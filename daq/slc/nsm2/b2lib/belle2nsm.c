/* ---------------------------------------------------------------------- *\
   belle2nsm.c

   NSM client library for Belle II experiment, including definitions of
   Belle II specific message definitions.  All lower level functions of
   nsmlib2.c are encapsulated in belle2nsm.c.

   All external library functions have prefix "b2nsm_".

   20131230 1918 strerror fix, initnet fix, stdint fix, bridge fix
   20140103 1919 more text into log for b2nsm_ok
   20140106 1921 wrapptr added
   20140124 1925 anonymous node, forgotten uprcase in openmem
   20140305 1927 allow null state in b2nsm_ok
   20140305 1929 checkpoints, no fprintf
   20140306 1930 logfp again, but write instead of fwrite/fprintf
   20140902 1935 memset fix
   20140921 1940 flushmem
   20160420 1946 debugflag separately from corelib
   20180121 1957 b2nsm_nodename is added
\* ---------------------------------------------------------------------- */

const char *belle2nsm_version = "belle2nsm 1.9.57";

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <nsm2/nsm2.h>
#include <nsm2/nsmlib2.h>
#include "belle2nsm.h"

/* checkpoint of signal handler to be studied with gdb
   0: never called, -1: done, 1..1000 user checkpoint, 1001.. corelib/b2lib
 */
extern int  nsmlib_currecursive;
#define DBS(nsmc,val) nsmlib_checkpoint(0,val)
#define DBGFLG(i) (b2nsm_debugflag&(1<<(i)))

NSMcontext *nsm = 0;
static int b2nsm_errc;
FILE *logfp = 0;
static int b2nsm_debugflag = 0;

typedef struct b2nsm_struct {
  char default_dest[32];
  char state[32];
} b2nsm_t;

/* -- xuprcpy ----------------------------------------------------------- */
static void
xuprcpy(char *upr, const char *str, int n)
{
  int i;
  for (i=0; i<n-1 && str[i]; i++) upr[i] = toupper(str[i]);
  upr[i] = 0;
}
/* -- xlogtime ---------------------------------------------------------- */
static const char *
xt()
{
  struct timeval now;
  struct tm *cur;
  static char buf[32];
  gettimeofday(&now, 0);
  cur = localtime((time_t *)&now.tv_sec);
  sprintf(buf, "%02d:%02d:%02d.%03d ",
	  cur->tm_hour, cur->tm_min, cur->tm_sec, (int)now.tv_usec/1000);
  return buf;
}
/* -- b2nsm_addincpath -------------------------------------------------- */
int
b2nsm_addincpath(const char *path)
{
  return nsmlib_addincpath(path);
}
/* -- b2nsm_nodename ---------------------------------------------------- */
const char *
b2nsm_nodename(int nodeid)
{
  return nsmlib_nodename(nsm, nodeid);
}
/* -- b2nsm_nodeid ------------------------------------------------------ */
int
b2nsm_nodeid(const char *nodename)
{
  char nodename_uprcase[NSMSYS_NAME_SIZ + 1];
  xuprcpy(nodename_uprcase, nodename, NSMSYS_NAME_SIZ + 1);
  return nsmlib_nodeid(nsm, nodename_uprcase);
}
/* -- b2nsm_nodepid ----------------------------------------------------- */
int 
b2nsm_nodepid(const char *nodename)
{
  int inod = b2nsm_nodeid(nodename);
  if (inod < 0) return -1;
  return ntohl(nsm->sysp->nod[inod].nodpid);
}
/* -- b2nsm_loghook ----------------------------------------------------- */
int
b2nsm_loghook(NSMmsg *msg, NSMcontext *nsmc)
{
  DBS(nsmc,2000);
  
  if (logfp && DBGFLG(0)) {
    int    i;
    int    npar = msg->npar;
    int   *pars = msg->pars;
    int    len  = msg->len;
    const char *datp = msg->datap;
    
    DBS(nsmc,2002);
    nsmlib_log("%s%s%s<=%s",
               xt(), nsmc->hookptr ? (const char *)nsmc->hookptr : "",
               nsmlib_reqname(nsmc, msg->req),
               nsmlib_nodename(nsmc, msg->node));

    DBS(nsmc,2004);
    for (i = 0; i < 3 && i < npar; i++) {
      nsmlib_log("%s%d%s", i==0 ? " (" : "", pars[i],
	      i==npar-1 ? ")" : (i == 2 ? "...)" : ",") );
    }
    DBS(nsmc,2006);
    for (i = 0; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      nsmlib_log("%s%c%s", i==0 ? " " : "", datp[i], i==79 ? "..." : "");
    }
    DBS(nsmc,2008);
    if (i < 79 && datp && datp[i] == 0 && len > i+1 && isprint(datp[i+1])) {
      nsmlib_log(" ");
    }
    DBS(nsmc,2010);
    for (i++; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      nsmlib_log("%c%s", datp[i], i==79 ? "..." : "");
    }
    DBS(nsmc,2012);
    nsmlib_log("\n");
    DBS(nsmc,2014);
    
    if (nsmlib_currecursive <= 1) nsmlib_logflush();

    DBS(nsmc,2016);
  }
  return 0;
}
/* -- b2nsm_checkpoint -------------------------------------------------- */
void
b2nsm_checkpoint(NSMcontext *nsmc, int val)
{
  nsmlib_checkpoint(nsmc, val);
}
/* -- b2nsm_debuglevel -------------------------------------------------- */
int
b2nsm_debuglevel(int val)
{
  int prev = b2nsm_debugflag & 0xffff;
  b2nsm_debugflag = val & 0xffff;
  return prev | (nsmlib_debuglevel(val >> 16) << 16);
}
/* -- b2nsm_logging ----------------------------------------------------- */
void
b2nsm_logging(FILE *fp)
{
  logfp = fp;
  nsmlib_logging(fp);
  if (nsm) {
    nsm->hook = b2nsm_loghook;
    nsm->hookptr = 0;
  }
}
/* -- b2nsm_logging2 ---------------------------------------------------- */
void
b2nsm_logging2(FILE *fp, const char *prefix)
{
  logfp = fp;
  nsmlib_logging(fp);
  if (nsm) {
    nsm->hook = b2nsm_loghook;
    nsm->hookptr = (const void *)prefix;
  }
}
/* -- b2nsm_context ----------------------------------------------------- */
void
b2nsm_context(NSMcontext *context)
{
  nsm = context;
}
/* -- b2nsm_strerror ---------------------------------------------------- */
const char *
b2nsm_strerror()
{
  switch (b2nsm_errc) {
  case NSMEALLOC:
    return "cannot alloc";
  default:
    return nsmlib_strerror(nsm);
  }
}
/* -- b2nsm_callback ---------------------------------------------------- */
int
b2nsm_callback(const char *name, NSMcallback_t callback)
{
  char name_uprcase[NSMSYS_NAME_SIZ+1];
  int ret;
  int oldsig;
  
  if (! nsm) {
    if (logfp) {
      nsmlib_log("NSM is not initialized\n");
      nsmlib_logflush();
    }
    return -1;
  }

  xuprcpy(name_uprcase, name, NSMSYS_NAME_SIZ+1);
  if (nsmlib_register_request(nsm, name) < 0) return -1;
  
  ret = nsmlib_callback(nsm, name, callback, NSMLIB_FNSTD);

  if (! logfp) return ret;

  if (ret < 0) {
    nsmlib_log("%scallback(%s) registration failed: %s\n",
	       xt(), name, b2nsm_strerror());
  } else {
    if (DBGFLG(1)) nsmlib_log("%scallback(%s) registered\n", xt(), name);
  }
  nsmlib_logflush();

  return ret;
}	     
/* -- b2nsm_sendany ----------------------------------------------------- */
int
b2nsm_sendany(const char *node, const char *req, int npar, int32_t *pars,
	      int len, const char *datp, const char *caller)
{
  int ret;
  char node_uprcase[NSMSYS_NAME_SIZ+1];
  char req_uprcase[NSMSYS_NAME_SIZ+1];
  int oldsig;

  if (! nsm) return -1;
  xuprcpy(node_uprcase, node, NSMSYS_NAME_SIZ+1);
  xuprcpy(req_uprcase,  req, NSMSYS_NAME_SIZ+1);
  ret = nsmlib_sendreq(nsm, node_uprcase, req_uprcase, npar, pars, len, datp);

  if (! logfp) return ret;

  if (ret < 0) {
    nsmlib_log("%s%s=>%s %s failed: %s\n",
	    xt(), req, node, caller, b2nsm_strerror());
  } else {
    int i;
    nsmlib_log("%s%s=>%s", xt(), req, node);
    for (i = 0; i<3 && i<npar; i++) {
      nsmlib_log("%s%d%s", i==0 ? " (" : "", pars[i],
	      i==npar-1 ? ")" : (i == 2 ? "...)" : ",") );
    }
    for (i = 0; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      nsmlib_log("%s%c%s", i==0 ? " " : "", datp[i], i==79 ? "..." : "");
    }
    if (i < 79 && datp && datp[i] == 0 && len > i+1 && isprint(datp[i+1])) {
      nsmlib_log(" ");
    }
    for (i++; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      nsmlib_log("%c%s", datp[i], i==79 ? "..." : "");
    }
    nsmlib_log("\n");
  }
  
  return ret;
}
/* -- b2nsm_sendreq ----------------------------------------------------- */
int
b2nsm_sendreq(const char *node, const char *req, int npar, int32_t *pars)
{
  return b2nsm_sendany(node, req, npar, pars, 0, 0, "sendreq");
}
/* -- b2nsm_ok ---------------------------------------------------------- *\
   newstate may be 0 to keep the same state
\* ---------------------------------------------------------------------- */
int
b2nsm_ok(NSMmsg *msg, const char *newstate, const char *fmt, ...)
{
  va_list ap;
  const char *node = nsmlib_nodename(nsm, msg->node);
  char buf[32+256];
  char *str = 0;
  int  len;
  int  pars[2];
  
  if (! nsm || ! msg) return -1;

  if (newstate) {
    if ((len = strlen(newstate) + 1) > 32) return -1;
    strcpy(buf, newstate);
    strcpy(((b2nsm_t *)nsm->wrapptr)->state, newstate);
  } else {
    strcpy(buf, ((b2nsm_t *)nsm->wrapptr)->state);
    len = strlen(buf) + 1;
  }
  
  if (fmt) {
    va_start(ap, fmt);
    vsnprintf(buf + len, 256, fmt, ap);
    va_end(ap);
    str = buf + len;
    len += strlen(buf + len) + 1;
  }

  pars[0] = msg->req;
  pars[1] = msg->seq;
  
  return b2nsm_sendany(node, "OK", 2, pars, len, buf, "ok");
}
/* -- b2nsm_error ------------------------------------------------------- */
int
b2nsm_error(NSMmsg *msg, const char *fmt, ...)
{
  va_list ap;
  const char *node = nsmlib_nodename(nsm, msg->node);
  char buf[256];
  int  len;
  int  pars[2];
  
  if (! nsm) return -1;
  if (! msg || ! fmt) return -1;

  if (fmt) {
    va_start(ap, fmt);
    vsnprintf(buf, 256, fmt, ap);
    va_end(ap);
    len = strlen(buf) + 1;
  } else {
    strcpy(buf, "(no message)");
  }

  pars[0] = msg->req;
  pars[1] = msg->seq;
  
  return b2nsm_sendany(node, "ERROR", 2, pars, len, buf, "error");
}
/* -- b2nsm_readmem ----------------------------------------------------- */
int
b2nsm_readmem(void *buf, const char *dat, const char *fmt, int rev)
{
  char dat_uprcase[NSMSYS_NAME_SIZ+1];
  int  ret;
  if (! nsm) return 0;
  xuprcpy(dat_uprcase, dat, NSMSYS_NAME_SIZ+1);
  ret = nsmlib_readmem(nsm, buf, dat_uprcase, fmt, rev);

  if (! logfp) return ret;
  if (ret < 0) {
    nsmlib_log("%sreadmem(%s,rev.%d) failed: %s\n",
	       xt(), dat, rev, b2nsm_strerror());
  }
  return ret;
}
/* -- b2nsm_statmem ----------------------------------------------------- */
int
b2nsm_statmem(const char *dat, char *fmtbuf, int buflen)
{
  char dat_uprcase[NSMSYS_NAME_SIZ+1];
  int ret;
  if (! nsm) return 0;
  xuprcpy(dat_uprcase, dat, NSMSYS_NAME_SIZ+1);
  ret = nsmlib_statmem(nsm, dat_uprcase, fmtbuf, buflen);

  if (! logfp) return ret;

  if (ret < 0) {
    nsmlib_log("%sstatmem(%s) failed: %s\n",
	       xt(), dat, b2nsm_strerror());
  } else if (ret == 0) {
    nsmlib_log("%sstatmem(%s) no fmt\n", xt(), dat);
  } else {
    nsmlib_log("%sstatmem(%s) fmt %s\n", xt(), dat, fmtbuf);
  }
  return ret;
}
/* -- b2nsm_openmem ----------------------------------------------------- */
void *
b2nsm_openmem(const char *dat, const char *fmt, int rev)
{
  char dat_uprcase[NSMSYS_NAME_SIZ+1];
  void *ptr;
  if (! nsm) return 0;
  xuprcpy(dat_uprcase, dat, NSMSYS_NAME_SIZ+1);
  ptr = nsmlib_openmem(nsm, dat_uprcase, fmt, rev);

  if (! logfp) return ptr;

  if (! ptr) {
    nsmlib_log("%sopenmem(%s,rev.%d) failed: %s\n",
	       xt(), dat, rev, b2nsm_strerror());
  } else {
    nsmlib_log("%sopenmem(%s,rev.%d) at %p\n", xt(), dat, rev, ptr);
  }
  return ptr;
}
/* -- b2nsm_allocmem ---------------------------------------------------- */
void *
b2nsm_allocmem(const char *dat, const char *fmt, int rev, float cycle)
{
  char dat_uprcase[NSMSYS_NAME_SIZ+1];
  void *ptr;
  if (! nsm) return 0;
  xuprcpy(dat_uprcase, dat, NSMSYS_NAME_SIZ+1);
  ptr = nsmlib_allocmem(nsm, dat_uprcase, fmt, rev, cycle);

  if (! logfp) return ptr;

  if (! ptr) {
    nsmlib_log("%sallocmem(%s,rev.%d) failed: %s\n",
	       xt(), dat, rev, b2nsm_strerror());
  } else {
    nsmlib_log("%sallocmem(%s,rev.%d) at %p\n", xt(), dat, rev, ptr);
  }
  return ptr;
}
/* -- b2nsm_flushmem ---------------------------------------------------- */
int
b2nsm_flushmem(const void *ptr, int siz)
{
  int ret;
  if (! nsm) return 0;
  ret = nsmlib_flushmem(nsm, ptr, siz);

  if (logfp && ret < 0) {
    nsmlib_log("%sflushmem(%x,%d) failed (ret=%d): %s\n",
	       xt(), ptr, siz, ret, b2nsm_strerror());
  }
  return ret;
}
/* -- b2nsm_wait -------------------------------------------------------- */
int
b2nsm_wait(float timeout)
{
  /* int wait_msec = (int)(timeout * 1000); */
  int wait_usec = (int)(timeout * 1000);
  char buf[NSM_TCPMSGSIZ]; /* should not be static */
  NSMcontext *nsmc = nsmlib_selectc(0, wait_usec); /* usesig = 0 */
  NSMcontext *nsmsav;

  if (! nsmc) return 0;

  /* receive */
  if (nsmlib_recv(nsmc, (NSMtcphead *)buf, 1000) <= 0) { /* 1 sec */
    return -1;
  }

  /* callback function */
  nsmsav = nsm;
  nsm = nsmc;
  nsmlib_call(nsmc, (NSMtcphead *)buf);
  nsm = nsmsav;
  return 1;
}
/* -- b2nsm_init2 ------------------------------------------------------- */
/*    node is anonymous when nodename = 0                                 */
/* ---------------------------------------------------------------------- */
NSMcontext *
b2nsm_init2(const char *nodename, int usesig,
	    const char *hostname, int port, int shmkey)
{
  char nodename_uprcase[NSMSYS_NAME_SIZ+1];
  b2nsm_errc = 0;
  if (nodename) {
    xuprcpy(nodename_uprcase, nodename, NSMSYS_NAME_SIZ+1);
    nsm = nsmlib_init(nodename_uprcase, hostname, port, shmkey);
  } else {
    nsm = nsmlib_init(nodename, hostname, port, shmkey);
  }
  if (nsm == 0) return 0;
  nsmlib_usesig(nsm, usesig);
  if (logfp) nsm->hook = b2nsm_loghook;
  nsm->wrapptr = (void *)malloc(sizeof(b2nsm_t));
  if (! nsm->wrapptr) {
    free(nsm);
    nsm = 0;
    b2nsm_errc = NSMEALLOC;
  }
  memset((char *)nsm->wrapptr, 0, sizeof(b2nsm_t));
  strcpy(((b2nsm_t *)nsm->wrapptr)->state, "UNKNOWN");
  
  return nsm;
}
/* -- b2nsm_init -------------------------------------------------------- */
NSMcontext *
b2nsm_init(const char *nodename)
{
  return b2nsm_init2(nodename, 1, 0, 0, 0);
}
/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
