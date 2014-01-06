/* ---------------------------------------------------------------------- *\
   belle2nsm.c

   NSM client library for Belle II experiment, including definitions of
   Belle II specific message definitions.  All lower level functions of
   nsmlib2.c are encapsulated in belle2nsm.c.

   All external library functions have prefix "b2nsm_".

   20131230 1918 strerror fix, initnet fix, stdint fix, bridge fix
   20140103 1919 more text into log for b2nsm_ok
   20140106 1921 wrapptr added
\* ---------------------------------------------------------------------- */

const char *belle2nsm_version = "belle2nsm 1.9.21";

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "nsm2.h"
#include "nsmlib2.h"
#include "belle2nsm.h"

NSMcontext *nsm = 0;
static int b2nsm_errc;
static FILE *logfp = 0;

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
  if (logfp) {
    int i;
    int    npar = msg->npar;
    int   *pars = msg->pars;
    int    len  = msg->len;
    const char *datp = msg->datap;
    
    fprintf(logfp, "%s%s%s<=%s",
	    xt(), nsmc->hookptr ? (const char *)nsmc->hookptr : "",
	    nsmlib_reqname(nsmc, msg->req),
	    nsmlib_nodename(nsmc, msg->node));

    for (i = 0; i < 3 && i < npar; i++) {
      fprintf(logfp, "%s%d%s", i==0 ? " (" : "", pars[i],
	      i==npar-1 ? ")" : (i == 2 ? "...)" : ",") );
    }
    for (i = 0; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      fprintf(logfp, "%s%c%s", i==0 ? " " : "", datp[i], i==79 ? "..." : "");
    }
    if (i < 79 && datp && datp[i] == 0 && len > i+1 && isprint(datp[i+1])) {
      fprintf(logfp, " ");
    }
    for (i++; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      fprintf(logfp, "%c%s", datp[i], i==79 ? "..." : "");
    }
    fprintf(logfp, "\n");
    fflush(logfp);
  }
  return 0;
}
/* -- b2nsm_logging ----------------------------------------------------- */
int
b2nsm_debuglevel(int val)
{
  return nsmlib_debuglevel(val);
}
/* -- b2nsm_logging ----------------------------------------------------- */
void
b2nsm_logging(FILE *fp)
{
  logfp = fp;
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
  if (! nsm) {
    if (logfp) fprintf(logfp, "NSM is not initialized");
    return -1;
  }

  xuprcpy(name_uprcase, name, NSMSYS_NAME_SIZ+1);
  if (nsmlib_register_request(nsm, name) < 0) return -1;
  
  ret = nsmlib_callback(nsm, name, callback, NSMLIB_FNSTD);

  if (! logfp) return ret;
  if (ret < 0) {
    fprintf(logfp, "%scallback(%s) registration failed: %s\n",
	    xt(), name, b2nsm_strerror());
  } else {
    fprintf(logfp, "%scallback(%s) registered\n", xt(), name);
  }
  fflush(logfp);
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

  if (! nsm) return -1;
  xuprcpy(node_uprcase, node, NSMSYS_NAME_SIZ+1);
  xuprcpy(req_uprcase,  req, NSMSYS_NAME_SIZ+1);
  ret = nsmlib_sendreq(nsm, node_uprcase, req_uprcase, npar, pars, len, datp);

  if (! logfp) return ret;
  if (ret < 0) {
    fprintf(logfp, "%s%s=>%s %s failed: %s\n",
	    xt(), req, node, caller, b2nsm_strerror());
  } else {
    int i;
    fprintf(logfp, "%s%s=>%s", xt(), req, node);
    for (i = 0; i<3 && i<npar; i++) {
      fprintf(logfp, "%s%d%s", i==0 ? " (" : "", pars[i],
	      i==npar-1 ? ")" : (i == 2 ? "...)" : ",") );
    }
    for (i = 0; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      fprintf(logfp, "%s%c%s", i==0 ? " " : "", datp[i], i==79 ? "..." : "");
    }
    if (i < 79 && datp && datp[i] == 0 && len > i+1 && isprint(datp[i+1])) {
      fprintf(logfp, " ");
    }
    for (i++; datp && i < 80 && i < len && isprint(datp[i]); i++) {
      fprintf(logfp, "%c%s", datp[i], i==79 ? "..." : "");
    }
    fprintf(logfp, "\n");
  }
  fflush(logfp);
  return ret;
}
/* -- b2nsm_sendreq ----------------------------------------------------- */
int
b2nsm_sendreq(const char *node, const char *req, int npar, int32_t *pars)
{
  return b2nsm_sendany(node, req, npar, pars, 0, 0, "sendreq");
}
/* -- b2nsm_ok ---------------------------------------------------------- */
int
b2nsm_ok(NSMmsg *msg, const char *newstate, const char *fmt, ...)
{
  va_list ap;
  const char *node = nsmlib_nodename(nsm, msg->node);
  char buf[32+256];
  char *str = 0;
  int  len;
  int  pars[2];
  
  if (! nsm || ! msg || ! newstate) return -1;

  if ((len = strlen(newstate) + 1) > 32) return -1;
  strcpy(buf, newstate);
  if (nsm->wrapptr) {
    strcpy(((b2nsm_t *)nsm->wrapptr)->state, "UNKNOWN");
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
/* -- b2nsm_openmem ----------------------------------------------------- */
void *
b2nsm_openmem(const char *dat, const char *fmt, int rev)
{
  char dat_uprcase[NSMSYS_NAME_SIZ+1];
  void *ptr;
  if (! nsm) return 0;
  xuprcpy(dat_uprcase, dat, NSMSYS_NAME_SIZ+1);
  ptr = nsmlib_openmem(nsm, dat, fmt, rev);

  if (! logfp) return ptr;
  if (! ptr) {
    fprintf(logfp, "%sopenmem(%s,rev.%d) failed: %s\n",
	    xt(), dat, rev, b2nsm_strerror());
  } else {
    fprintf(logfp, "%sopenmem(%s,rev.%d) at %p\n", xt(), dat, rev, ptr);
  }
  fflush(logfp);
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
    fprintf(logfp, "%sallocmem(%s,rev.%d) failed: %s\n",
	    xt(), dat, rev, b2nsm_strerror());
  } else {
    fprintf(logfp, "%sallocmem(%s,rev.%d) at %p\n", xt(), dat, rev, ptr);
  }
  fflush(logfp);
  return ptr;
}
/* -- b2nsm_wait -------------------------------------------------------- */
int
b2nsm_wait(float timeout)
{
  int wait_msec = (int)(timeout * 1000);
  char buf[NSM_TCPMSGSIZ]; /* should not be static */
  NSMcontext *nsmc = nsmlib_selectc(0, wait_msec); /* usesig = 0 */
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
NSMcontext *
b2nsm_init2(const char *nodename, int usesig,
	    const char *hostname, int port, int shmkey)
{
  char nodename_uprcase[NSMSYS_NAME_SIZ+1];
  b2nsm_errc = 0;
  xuprcpy(nodename_uprcase, nodename, NSMSYS_NAME_SIZ+1);
  nsm = nsmlib_init(nodename_uprcase, hostname, port, shmkey);
  if (nsm == 0) return 0;
  nsmlib_usesig(nsm, usesig);
  if (logfp) nsm->hook = b2nsm_loghook;
  nsm->wrapptr = (void *)malloc(sizeof(b2nsm_t));
  if (! nsm->wrapptr) {
    free(nsm);
    nsm = 0;
    b2nsm_errc = NSMEALLOC;
  }
  memset((char *)nsm->wrapptr, sizeof(b2nsm_t), 0);
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
b// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
