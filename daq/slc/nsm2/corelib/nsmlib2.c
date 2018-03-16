/* ---------------------------------------------------------------------- *\
   nsmlib2.c

   NSM client library
   core part, independent of general usage

   All external library functions have prefix "nsmlib_".

   20131216 1912 sa_flags fix (work works with gcc 4.1.2)
   20131230 1918 strerror fix, initnet fix, stdint fix, bridge fix
   20140104 1919 strerror rearranged
   20140105 1921 fix for incorrect errc in register_request
   20140108 1922 initcli error message fix
   20140124 1925 anonymous node
   20140304 1926 abort when new revision data is started elsewhere
   20140305 1929 maxrecursive and checkpoints, no fprintf
   20140306 1930 logfp again, but write instead of fwrite/fprintf
   20140516 1931 ip address from shm if not specified
   20140614 1932 sprintf ip address if host is missing
   20140614 1933 use nodhash for nsmlib_nodeid
   20140902 1935 memset fix
   20140903 1937 nsmparse fix (see nsmparse.c)
   20140917 1939 skip revision check for -1
   20140921 1940 flushmem
   20180118 1954 nsmlib_send timeout problem fix
   20180120 1956 nsmlib_selectu for usec, nsmlib_select as static
\* ---------------------------------------------------------------------- */

const char *nsmlib2_version   = "nsmlib2 1.9.56";

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h> /* for struct hostent */
#include <net/if.h> /* for struct ifconf, ifreq */
#include <sys/socket.h> /* for ntoha etc */
#include <netinet/in.h> /* for ntoha etc */
#include <arpa/inet.h>  /* for ntoha etc */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include "nsm2.h"
#include "nsmlib2.h"

#define NSM2_PORT 8120 /* chosen as it corresponds to 0x2012
			  (2012 is the year started writing NSM2) */
#define NSMENV_HOST   "NSM2_HOST"
#define NSMENV_PORT   "NSM2_PORT"
#define NSMENV_SHMKEY "NSM2_SHMKEY"

#define NSMLIB_MAXRECURSIVE 256    /* default value */

static NSMcontext *nsmlib_context = 0;  /* top of the chain */
static FILE *nsmlib_logfp = 0;
static int   nsmlib_loglen = 0;
static char  nsmlib_logbuf[4096];
static char *nsmlib_incpath = 0;
static char  nsmlib_errs[1024];    /* error string in case of NSMEUNEXPECTED */
static int   nsmlib_errc = 0;      /* context independent error code */
static int   nsmlib_port = 0;      /* error port without context */

static int   nsmlib_debugflag = 0; /* context independent error code */
static sigset_t nsmlib_sigmask;
static int   nsmlib_sig;

static int  nsmlib_maxrecursive;
int  nsmlib_currecursive;
static int *nsmlib_checkpoints;
static int  nsmlib_nskipped;
static NSMtcphead nsmlib_lastskipped;

#define ADDR_N(a)    ((uint32_t)((a).sin_addr.s_addr))
/*
#define VSPRINTF(buf,fmt,ap)  va_start(ap,fmt);vsprintf(buf,fmt,ap);va_end(ap)
#define VFPRINTF(fp,fmt,ap)   va_start(ap,fmt);vfprintf(fp,fmt,ap);va_end(ap)
*/
#define LOGEND (nsmlib_logbuf + nsmlib_loglen)
#define LOGCLR nsmlib_loglen = 0
#define LOGSIZ (sizeof(nsmlib_logbuf) - nsmlib_loglen - 1)
#define LOGFWD nsmlib_loglen += strlen(LOGEND)
#define LOGCAT(s) strcpy(LOGEND, s); LOGFWD

#define VSNPRINTF(buf,s,f,ap) va_start(ap,f);vsnprintf(buf,s,f,ap);va_end(ap)
#define SOCKOPT(s,o,v)        setsockopt(s,SOL_SOCKET,o,(char *)&(v),sizeof(v))

#define MEMPOS(base,ptr) ((char *)(ptr) - (char *)(base))
#define MEMPTR(base,off) ((char *)(base) + off)

#define ASSERT nsmlib_assert
#define ERROR  nsmlib_error
#define DBG    nsmlib_debug
#define LOG    nsmlib_lognl

/* checkpoint of signal handler to be studied with gdb
   0: never called, -1: done, 1..1000 user checkpoint, 1001.. corelib/b2lib
 */
#define DBS(nsmc,val) nsmlib_checkpoints[nsmlib_currecursive] = (val)

int nsmlib_hash(NSMsys *, int32_t *hashtable, int hashmax,
		const char *key, int create);
char * nsmlib_parse(const char *datname, int revision, const char *incpath);
const char *nsmlib_parseerr(int *errcode);

/* -- time1ms -------------------------------------------------------- */
static uint64
time1ms()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (uint64)tv.tv_sec * 1000 + tv.tv_usec / 1000; /* in 1msec unit */
}
/* -- htonll --------------------------------------------------------- */
static uint64
htonll(uint64 h)
{
  static int n42 = 0;
  if (n42 == 0) n42 = htons(42); /* 42 is the answer */
  if (n42 == 42) {
    return h;
  } else {
    uint64 n;
    int32_t *hp = (int32_t *)&h;
    int32_t *np = (int32_t *)&n;
    np[0] = htonl(hp[1]);
    np[1] = htonl(hp[0]);
    return n;
  }
}
/* -- nsmlib_logflush ------------------------------------------------ */
void
nsmlib_logflush()
{
  /*
    (fwrite version does not properly work at 1.9.30)
    fwrite(nsmlib_logbuf, nsmlib_loglen, 1, nsmlib_logfp);
    fflush(nsmlib_logfp);
  */
  write(fileno(nsmlib_logfp), nsmlib_logbuf, nsmlib_loglen);
  LOGCLR;
}
/* -- nsmlib_assert -------------------------------------------------- */
static void
nsmlib_assert(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;

  if (LOGSIZ < 256) {
    LOGCLR;
    LOGCAT("(output buffer shrinked)\n");
  }
  
  LOGCAT("ASSERT: ");
  VSNPRINTF(LOGEND, LOGSIZ, fmt, ap);
  LOGFWD;
  LOGCAT("\n");
  nsmlib_logflush();
  exit(1);
}
/* -- nsmlib_error --------------------------------------------------- */
static void
nsmlib_error(const char *fmt, ...)
{
  va_list ap;

  if (! nsmlib_logfp) return;

  if (LOGSIZ < 256) {
    LOGCLR;
    LOGCAT("(output buffer shrinked)\n");
  }
  
  LOGCAT("ERROR: ");
  VSNPRINTF(LOGEND, LOGSIZ, fmt, ap);
  LOGFWD;
  LOGCAT("\n");
  nsmlib_logflush();
}
/* -- nsmlib_debuglevel ---------------------------------------------- */
int
nsmlib_debuglevel(int val)
{
  int prev = nsmlib_debugflag;
  nsmlib_debugflag = val;
  return prev;
}
/* -- nsmlib_debug --------------------------------------------------- */
static void
nsmlib_debug(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;
  if (! nsmlib_debugflag) return;

  if (LOGSIZ > 256) {
    LOGCAT("[DBG] ");
    VSNPRINTF(LOGEND, LOGSIZ, fmt, ap);
    LOGFWD;
    LOGCAT("\n");
  }
}
/* -- nsmlib_lognl --------------------------------------------------- */
void
nsmlib_lognl(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;
  if (LOGSIZ > 256) {
    VSNPRINTF(LOGEND, LOGSIZ, fmt, ap);
    LOGFWD;
    LOGCAT("\n");
  }
}
/* -- nsmlib_log ----------------------------------------------------- */
void
nsmlib_log(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;
  if (LOGSIZ > 256) {
    VSNPRINTF(LOGEND, LOGSIZ, fmt, ap);
    LOGFWD;
  }
}
/* -- nsmlib_logging ------------------------------------------------- */
void
nsmlib_logging(FILE *logfp)
{
  nsmlib_logfp = logfp;
}

/* -- nsmlib_checkpoint ---------------------------------------------- *\
   for user function, between 1 and 999 (outside range becomes 1000)
\* ------------------------------------------------------------------- */
void
nsmlib_checkpoint(NSMcontext *nsm, int val)
{
  DBS(nsm, val);
}
/* -- nsmlib_strerror ------------------------------------------------ */
const char *
nsmlib_strerror(NSMcontext *nsmc)
{
  static char buf[256];
  char *syserr = 0;
  int         errc = nsmc ? nsmc->errc : nsmlib_errc;
  const char *errs = nsmc ? nsmc->errs : nsmlib_errs;

  switch (errc) {
  case NSMENOERR: return nsmc ? "no error" : "NSM is not initialized";
  case NSMEALLOC: syserr = "malloc"; break;
  case NSMESOCKET: syserr = "socket"; break;
  case NSMESOCKDGRAM: syserr = "socket(sock_dgram)"; break;
  case NSMEGIFCONF: syserr = "ioctl(SIOCGIFCONF)"; break;
  case NSMEGIFFLAGS: syserr = "ioctl(SIOCGIFFLAGS)"; break;
  case NSMESOCKREUSE: syserr = "so_sockreuse"; break;
  case NSMESOCKSNDBUF: syserr = "so_sndbuf";    break;
  case NSMESOCKRCVBUF: syserr = "so_rcvbuf";    break;
  case NSMERDSELECT: syserr = "select(read)"; break;
  case NSMERDUID:    syserr = "uid read error"; break;
  case NSMENOPIPE:   syserr = "pipe"; break;
  case NSMESELECT:   syserr = "select"; break;
  case NSMETIMEOUT:  return "select timeout";
  case NSMEWRITE:    syserr = "write(socket)"; break;
  case NSMEUNEXPECTED: return errs;
  case NSMECLOSED:    return "NSMD connection closed";
  case NSMEPIPEREAD:  syserr = "read(pipe)"; break;
  case NSMEPIPEWRITE: syserr = "write(pipe)"; break;
  case NSMEDATID: return errs;
    /* possible errors by user parameters */
  case NSMENOMASTER:  return "no NSM master";
  case NSMEINVNAME:   return "invalid name"; /* from nsmd2, shouldn't happen */
  case NSMEINVPAR:    return "invalid parameter";
  case NSMENODEEXIST: return errs; /* "node already exist" from nsmd2 */
  case NSMEFULNODE:   return errs; /* "no more NSM node"   from nsmd2 */
  case NSMENODEST:    return "destination node is gone"; /* why unsed? */
  case NSMEINVFMT:    return "invalid data format"; /* from nsmd2 */
  case NSMEMEMEXIST:  return "data alraedy exists"; /* why unused? */
  case NSMENOMOREMEM: return "no more data area"; /* why unused? */
  case NSMEOPENED:    return "already opened"; /* from nsmd2 */
  case NSMENODENAME:  return "invalid node name";
  case NSMENODELONG:  return "node name too long"; /* why unused? */
  case NSMEHOSTNAME:  return "invalid hostname";
  case NSMEALREADYP:  return "host:port already in use";
  case NSMEALREADYS:  return "shm key already in use";
  case NSMEALREADYH:  return "host:port already in use"; /* deprecated */
  case NSMENOIF:      return "network interface not found";
  case NSMENONSMD:    return "orphan shared memory with no nsmd2 running";
  case NSMECONNECT:   syserr = "cannot connect to nsmd2"; break;
  case NSMENOUID:     return "uid not received";
  case NSMERDCLOSE:   return "uid not fully received";
  case NSMEACCESS:    return "nsmd2 is running under different uid";
  case NSMESHMGETSYS:
    if (errno == ENOENT) {
      sprintf(buf, "no nsmd2 or shared memory with NSM2_PORT=%d", nsmlib_port);
      return buf;
    }
    syserr = "cannot open NSMsys shared memory"; break;
  case NSMESHMGETMEM:
    if (errno == EACCES) {
      strcpy(buf, "nsmd2 shared memory must have either the same uid or gid");
      return buf;
    }
    syserr = "cannot open NSMmem shared memory"; break;
  case NSMESHMATSYS: syserr = "shmat(NSMsys)"; break;
  case NSMESHMATMEM: syserr = "shmat(NSMmem)"; break;
  case NSMENOINIT:  return "NSM is not initialized yet";
  case NSMEPERM:    return "operation not allowed for anonymous node";
  case NSMEINVDATA: return "invalid send data parameter";
  case NSMEINVFUNC: return "invalid callback function type";
  case NSMEMAXFUNC: return "no more callback functions";
  case NSMENOMEM:   return errs; /* openmem */
  case NSMEBADFMT:  return errs; /* openmem */
  case NSMEBADREV:  return errs; /* openmem/allocmem */
  case NSMEPARSE:   return errs; /* openmem/allocmem */
  case NSMEINVPTR:  return "invalid pointer";

  default:
    if (*errs) return errs;
    if (! nsmc) {
      sprintf(buf, "NSM is not initialized (errcode=%d)", errc);
    } else {
      sprintf(buf, "undefined error code (errcode=%d)", errc);
    }
    return buf;
  }
  
  if (syserr) {
    sprintf(buf, "%s: %s", syserr, strerror(errno));
  }
  return buf;
}
/* -- nsmlib_atoi ---------------------------------------------------- */
static int
nsmlib_atoi(const char *a, int def)
{
  if (! a || ! *a) return def;
  if (! (isdigit(*a) || (*a == '-' && isdigit(*(a+1))))) {
    return def;
  } else {
    return (int)strtol(a,0,0);
  }
}

/* -- nsmlib_addincpath ---------------------------------------------- */
int
nsmlib_addincpath(const char *path)
{
  if (! path) {
    if (nsmlib_incpath) free(nsmlib_incpath);
    nsmlib_incpath = 0;
    return 0;
  } else {
    /*
       If "path" is not a directory, take the string up to the last '/'.
       This is the way to easily add the path of the program, by
       nsmlib_addincpath(argv[0])
    */
    const char *p;
    int len = nsmlib_incpath ? strlen(nsmlib_incpath) + 1 : 0;
    struct stat statbuf;
    while (stat(path, &statbuf) < 0) {
      if (errno != EINTR) return -1;
    }
    
    if (S_ISDIR(statbuf.st_mode)) {
      len += strlen(path) + 1;
      p = path + len;
    } else {
      if (! (p = strrchr(path, '/'))) return -1;
      len += (p - path) + 1;
    }
    char *q = (char *)malloc(len);
    if (! q) ASSERT("nsmlib_addincpath can't malloc %d byte", len);
    if (nsmlib_incpath) {
      sprintf(q, "%s:", nsmlib_incpath);
    } else {
      *q = 0;
    }
    strncat(q, path, p - path);
    q[len-1] = 0;
    if (nsmlib_incpath) free(nsmlib_incpath);
    nsmlib_incpath = q;
  }
}

/* -- nsmlib_nodename ------------------------------------------------ */
const char *
nsmlib_nodename(NSMcontext *nsmc, int nodeid)
{
  int i;
  NSMsys *sysp;
  const char *namep;
  if (! nsmc || ! (sysp = nsmc->sysp)) return 0;

  if (nodeid == NSMSYS_MAX_NOD) return "(no-name)";
  if (nodeid < 0 || nodeid > NSMSYS_MAX_NOD) return "(invalid)";
  namep = sysp->nod[nodeid].name;
  if (! namep[0]) return 0;
  for (i=0; i <= NSMSYS_NAME_SIZ; i++) {
    if (namep[i] == 0) break;
    if (! isalnum(namep[i]) && namep[i] != '_') return "(broken)";
  }
  if (i > NSMSYS_NAME_SIZ) return "(broken)";
  return namep;
}

/* -- nsmlib_nodeid -------------------------------------------------- */
int
nsmlib_nodeid(NSMcontext *nsmc, const char *nodename)
{
  int i;
  NSMsys *sysp = nsmc ? nsmc->sysp : 0;
  const char *namep;
  int hash = nsmlib_hash(sysp, sysp->nodhash, NSMSYS_MAX_HASH, nodename, 0);
  NSMnod *nodp = (NSMnod *)MEMPTR(sysp, ntohl(sysp->nodhash[hash]));

  if (! nsmc || ! sysp || hash < 0)   return -1;
  if (! nodename || *nodename == '(') return -1;
  return (int)(nodp - sysp->nod);
}

/* -- nsmlib_checkif ------------------------------------------------- */
/*
   Check the ethernet interface and determine the IP address to use.
*/
static int
nsmlib_checkif(NSMcontext *nsmc, SOCKAD_IN *sap)
{
  int sock;
  struct ifconf ifc;
  struct ifreq *ifr;
  char buf[4096];
  int ifrgap;
  
  /* -- open socket -- */
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    ERROR("nsmlib_checkif: socket");
    return NSMESOCKDGRAM;
  }
  
  /* -- broadcast address (only for the first, except localhost) -- */
  memset(buf, 0, sizeof(buf));
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0) {
    ERROR("nsmlib_checkif: ioctl getconf");
    return NSMEGIFCONF;
  }
  
  for (ifr = ifc.ifc_req;
       (char *)ifr < (char *)ifc.ifc_req + ifc.ifc_len;
       ifr = (struct ifreq *)((char *)ifr + ifrgap)) {

    
#if defined(__FreeBSD__) || defined(MVME5100)
    /* This ugly code is taken from bind 4.8.x for FreeBSD */
    /* VxWorks on MVME5100 requires this, too */
#define xxx_max(a,b) ((a)>(b)?(a):(b))
#define xxx_size(p)  xxx_max((p).sa_len,sizeof(p))
    ifrgap = sizeof(ifr->ifr_name) + xxx_size(ifr->ifr_addr);
#else
    ifrgap = sizeof(*ifr);
#endif
    
    if (! ifr->ifr_name[0]) break;
    DBG("nsmlib_checkif: checking interface <%s>", ifr->ifr_name);
    
    if (ioctl(sock, SIOCGIFFLAGS, (char *)ifr) < 0) {
      ERROR("nsmlib_checkif: ioctl getflags");
      return NSMEGIFFLAGS;
      
    } else if ((ifr->ifr_flags & IFF_UP) == 0) {
      DBG("nsmlib_checkif: interface <%s> is down", ifr->ifr_name);
      
    } else if ((ifr->ifr_flags & IFF_BROADCAST) == 0 &&
               (ifr->ifr_flags & IFF_LOOPBACK) == 0) {
      DBG("nsmlib_checkif: interface <%s> does not support broadcast",
	  ifr->ifr_name);

    } else if (ioctl(sock, SIOCGIFADDR, (char *)ifr) < 0) {
      DBG("nsmlib_checkif: interface <%s> has no address", ifr->ifr_name);
      
    } else {
      SOCKAD_IN sa;
      memcpy(&sa, (SOCKAD_IN *)&ifr->ifr_addr, sizeof(sa));
    
      if (sa.sin_addr.s_addr != sap->sin_addr.s_addr) {
	DBG("nsmlib_checkif: address does not match for interface <%s>",
	    ifr->ifr_name);
      } else {
	close(sock);
	return 0;
      }
    }
  }

  DBG("nsmlib_checkif: cannot find network interface for %s",
      inet_ntoa(sap->sin_addr));
  close(sock);
  
  return NSMENOIF;
}
/* -- nsmlib_initnet ------------------------------------------------- */
/*
  since nsm-1931 second argument is unused.
 */
int
nsmlib_initnet(NSMcontext *nsmc, const char *unused, int port)
{
  struct hostent *hp;
  NSMcontext *nsmcp;
  int ret;
  int TRUE = 1;
  int size = NSM_TCPBUFSIZ;
  uid_t nsmd_euid;
  fd_set fdset;
  struct timeval tv;

  /* -- check process -- */
  if (kill(nsmc->sysp->pid, 0) < 0 && errno == ESRCH) {
    return NSMENONSMD;
  }
  
  /* -- port and host -- */
  memset(nsmc->hostname, 0, sizeof(nsmc->hostname));
  memset(&nsmc->sa, 0, sizeof(nsmc->sa));

  memcpy(&nsmc->sa.sin_addr, &nsmc->sysp->ipaddr, 4);
  hp = gethostbyaddr(&nsmc->sa.sin_addr, 4, AF_INET);
  if (hp) {
    strcpy(nsmc->hostname, hp->h_name);
  } else {
    sprintf(nsmc->hostname, "%d.%d.%d.%d",
	    ((const char *)&nsmc->sysp->ipaddr)[0],
	    ((const char *)&nsmc->sysp->ipaddr)[1],
	    ((const char *)&nsmc->sysp->ipaddr)[2],
	    ((const char *)&nsmc->sysp->ipaddr)[3]);
  }
  
  nsmc->sa.sin_family = AF_INET;
  nsmc->sa.sin_port   = htons((short)nsmc->port);

  /* -- check if this port is already used -- */
  for (nsmcp = nsmlib_context; nsmcp; nsmcp = nsmcp->next) {
    if (nsmc == nsmcp) continue;
    if (nsmc->shmkey     == nsmcp->shmkey)     return NSMEALREADYS;
    if (nsmc->port == nsmcp->port &&
	ADDR_N(nsmc->sa) == ADDR_N(nsmcp->sa)) return NSMEALREADYP;
  }
  
  /* -- open a socket -- */
  if ((nsmc->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return NSMESOCKET;

  /* -- check interface -- */
  if ((ret = nsmlib_checkif(nsmc, &nsmc->sa)) < 0) return ret;

  /* -- socket options -- */
  if (SOCKOPT(nsmc->sock, SO_REUSEADDR, TRUE) < 0) return NSMESOCKREUSE;
  if (SOCKOPT(nsmc->sock, SO_SNDBUF,    size) < 0) return NSMESOCKSNDBUF;
  if (SOCKOPT(nsmc->sock, SO_RCVBUF,    size) < 0) return NSMESOCKRCVBUF;

  /* -- connect -- */
  if (connect(nsmc->sock, (SOCKAD *)&nsmc->sa, sizeof(nsmc->sa)) < 0) {
    return NSMECONNECT;
  }

  /* -- should receive 4 bytes -- */
  FD_ZERO(&fdset);
  FD_SET(nsmc->sock, &fdset);
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  while (1) {
    ret = select(nsmc->sock+1, &fdset, 0, 0, &tv);
    if (ret <  0 && errno == EINTR) continue;
    if (ret <  0) return NSMERDSELECT;
    if (ret == 0) return NSMENOUID;
    break;
  }
  
  /* -- check whether the nsmd belongs to the same uid -- */
  while (1) {
    ret = read(nsmc->sock, &nsmd_euid, sizeof(nsmd_euid));
    if (ret < 0 && errno == EINTR) continue;
    if (ret < 0) return NSMERDUID;
    if (ret < sizeof(nsmd_euid)) return NSMERDCLOSE;
    break;
  }
  
  if (nsmd_euid != 0 && nsmd_euid != getuid())
    return NSMEACCESS;
  
  nsmc->initnet_done = 1;
  
  return 0;
}
/* -- nsmlib_initshm ------------------------------------------------- */
/*
  initshm has to be called before initnet
 */
int
nsmlib_initshm(NSMcontext *nsmc, int shmkey, int port)
{
  /* -- initshm is called before inetnet --*/
  nsmc->port   = port;

  /* -- environment variables for shmkey -- */
  if (! shmkey) shmkey = nsmlib_atoi(getenv(NSMENV_SHMKEY), port);
  nsmc->shmkey = shmkey;

  /* -- system shared memory (read-only) -- */
  DBG("nsmlib_initshm: key=%d size=%d", shmkey, sizeof(NSMsys));
  nsmc->sysid = shmget(shmkey, sizeof(NSMsys), 0444);
  if (nsmc->sysid < 0) {
    /* -- most likely nsmd is not running,
       -- further check is made in initnet -- */
    return NSMESHMGETSYS;
  }

  nsmc->sysp = (NSMsys *)shmat(nsmc->sysid, 0, SHM_RDONLY);
  if (nsmc->sysp == (NSMsys *)-1) return NSMESHMATSYS;

  /* -- data shared memory (read/write) -- */
  nsmc->memid = shmget(shmkey+1, sizeof(NSMmem), 0664);
  if (nsmc->memid < 0) return NSMESHMGETMEM;

  nsmc->memp = (NSMmem *)shmat(nsmc->memid, 0, 0);
  if (nsmc->memp == (NSMmem *)-1) return NSMESHMATMEM;

  /* -- done -- */
  nsmc->initshm_done = 1;
  return 0;
}
/* -- nsmlib_select -------------------------------------------------- */
static int /* for internal use only */
nsmlib_select(int fdr, int fdw, unsigned int msec)
{
  struct timeval tv;
  fd_set fdset;
  int ret;
  if (fdr) fdw = 0;
  FD_ZERO(&fdset);
  FD_SET(fdr + fdw, &fdset);
  tv.tv_sec  = msec / 1000;
  tv.tv_usec = (msec % 1000) * 1000;
  while (1) {
    ret = select(fdr+fdw+1, fdr ? &fdset : 0, fdr ? 0 : &fdset, 0, &tv);
    if (ret != -1 || (errno != EINTR && errno != EAGAIN)) break;
  }
  if (ret < 0) {
    ASSERT("nsmlib_select should not reach here - ret=%d", ret);
  }
  return ret;
}
/* -- nsmlib_selectu ------------------------------------------------- */
/*
  usesig = 1 when called from signal handler
             then nsmc->usesig=1 is taken
  usesig = 0 when called from the user program
             then nsmc->usesig<1 is taken
 */
NSMcontext *
nsmlib_selectc(int usesig, unsigned int msec)
{
  return nsmlib_selectu(usesig, msec, 0);
}
NSMcontext *
nsmlib_selectu(int usesig, unsigned int msec, unsigned int usec)
{
  fd_set fdset;
  struct timeval tv;
  int ret;
  int highest = -1;
  NSMcontext *nsmc;
  
  FD_ZERO(&fdset);
  for (nsmc = nsmlib_context; nsmc; nsmc = nsmc->next) {
    if (usesig != 0 && nsmc->usesig < 1) continue;
    if (usesig == 0 && nsmc->usesig > 0) continue;
    FD_SET(nsmc->sock, &fdset);
    if (highest < nsmc->sock + 1) highest = nsmc->sock + 1;
  }
  if (highest == -1) return 0;
  
  tv.tv_sec  = msec / 1000;
  tv.tv_usec = (msec % 1000) * 1000 + usec;
  while (1) {
    ret = select(highest, &fdset, 0, 0, &tv);
    if (ret != -1 || (errno != EINTR && errno != EAGAIN)) break;
  }
  if (ret > 0) {
    for (nsmc = nsmlib_context; nsmc; nsmc = nsmc->next) {
      if (FD_ISSET(nsmc->sock, &fdset)) return nsmc;
    }
    ASSERT("nsmlib_selectc should not reach here - no fd");
    return 0;
  }
  if (usesig > 0 && msec > 0 && ret <= 0) {
    LOG("nsmlib_selectc should not reach here - ret=%d msec=%d usesig=%d", ret, msec, nsmc->usesig);
  }
  return 0;
}
/* -- nsmlib_pipewrite ----------------------------------------------- */
/*
  write one int32_t from pipe, to synchronize and get the result
 */
static int
nsmlib_pipewrite(NSMcontext *nsmc, int32_t val)
{
  while (1) {
    int ret = write(nsmc->pipe_wr, &val, sizeof(int32_t));
    if (ret < 0 && errno == EINTR) continue;
    if (ret < 0) ASSERT("nsmlib_pipewrite %s", strerror(errno));
    break;
  }
  return 0;
}
/* -- nsmlib_piperead ------------------------------------------------ */
/*
  read one int32_t from pipe, to synchronize and get the result
 */
static int
nsmlib_piperead(NSMcontext *nsmc, int32_t *valp)
{
  DBG("piperead 1");
  while (1) {
    int ret = read(nsmc->pipe_rd, valp, sizeof(int32_t));
    if (ret < 0 && errno == EINTR) continue;
    if (ret < 0) return NSMEPIPEREAD;
    break;
  }
  DBG("piperead done");
  return 0;
}
/* -- nsmlib_queue --------------------------------------------------- */
int
nsmlib_queue(NSMcontext *nsmc, NSMtcphead *hp)
{
  int len = sizeof(NSMtcphead) + sizeof(int32_t)*hp->npar + htons(hp->len);
  NSMrecvqueue *p = malloc(sizeof(struct NSMtcpqueue *) + len);
  if (! p) return 0;
  
  NSMrecvqueue *q = nsmc->recvqueue;
  if (! q) {
    nsmc->recvqueue = p;
  } else {
    while (q->next) q = q->next;
    q->next = p;
  }
  p->next = 0;
  memcpy((char *)&p->h, (char *)hp, len);
  return 0;
}
/* -- nsmlib_recv ---------------------------------------------------- */
int
nsmlib_recv(NSMcontext *nsmc, NSMtcphead *hp, int wait_msec)
{
  int sock = nsmc->sock;
  int ret;
  int i;
  int recvlen = 0;
  int datalen = sizeof(NSMtcphead);
  char *bufp  = (char *)hp;

  DBG("recv 1");
  ret = nsmlib_select(sock, 0, wait_msec);
  DBG("recv 2");
  if (ret < 0) ASSERT("nsmlib_recv: should not reach here (1)");
  if (ret == 0) return 0; /* time out */

  for (i=0; i<2; i++) { /* i=0 for header / i=1 for data */
    DBG("recv 3 bufp=%x", bufp);
    while (recvlen < datalen) {
      DBG("recv 4");
      if (recvlen > 0) ret = nsmlib_select(sock, 0, 100); /* 1 msec */
      if (ret < 0)  ASSERT("nsmlib_recv: should not reach here (2)");
      if (ret == 0) ASSERT("nsmlib_recv: should not reach here (3)");
      if ((ret = read(sock, bufp+recvlen, datalen)) <= 0) {
	if (ret == -1 && errno == EINTR) continue;
	if (ret < 0) ASSERT("nsmlib_recv read error (1) %s", strerror(errno));
	return (nsmlib_errc = NSMECLOSED);
      }
      datalen -= ret;
      recvlen += ret;
    }

    DBG("recv 5 recvlen=%d", recvlen);
    /* for data */
    bufp += recvlen;
    recvlen = 0;
    datalen = hp->npar * sizeof(int32_t) + ntohs(hp->len);
  }
  DBG("recv 6 datalen=%d npar=%d len=%d", datalen, hp->npar, ntohs(hp->len));
  return sizeof(NSMtcphead) + datalen;
}
/* -- nsmlib_recvpar ------------------------------------------------- */
int
nsmlib_recvpar(NSMcontext *nsmc)
{
  char buf[NSM_TCPMSGSIZ];
  NSMtcphead *hp = (NSMtcphead *)buf;
  int32_t *hp_pars = (int32_t *)(buf + sizeof(NSMtcphead));
  int ret = 0;
  int val;

  DBG("recvpar 1");
  if (nsmc->usesig > 0) {
    DBG("recvpar 2");
    ret = nsmlib_piperead(nsmc, &val);
  } else {
    DBG("recvpar 3");
    int64 tlimit = time1ms() + 5000; /* 5 seconds */
    int wait_msec = 5000;
    while (1) {
      DBG("recvpar 4");
      if ((ret = nsmlib_recv(nsmc, hp, wait_msec)) <= 0) {
	if (ret == 0) {
	  sprintf(nsmc->errs, "recvpar: timeout");
	  ret = NSMEUNEXPECTED;
	}
	break;
      }
      DBG("recvpar 5");
	
      if (ntohs(hp->req) != nsmc->reqwait) {
	DBG("recvpar 6 req=%x wait=%x", ntohs(hp->req), nsmc->reqwait);
	if (nsmlib_queue(nsmc, hp) < 0) return -1;
      } else if (hp->npar < 1) {
	DBG("recvpar 7");
	sprintf(nsmc->errs, "recvpar: npar=0 for req=%x", nsmc->reqwait);
	ret = NSMEUNEXPECTED;
	break;
      } else {
	DBG("recvpar 8");
	val = htonl(hp_pars[0]);
	break;
      }
      wait_msec = (tlimit - time1ms());
    }
  }
  DBG("recvpar 9");
  nsmc->reqwait = 0;
  if (ret < 0) return nsmc->errc = ret;
  if (val < 0) nsmc->errc = val;
  return val;
}
/* -- nsmlib_call ---------------------------------------------------- */
void
nsmlib_call(NSMcontext *nsmc, NSMtcphead *hp)
{
  int i;
  NSMmsg msg;
  const char *recvbuf = (char *)hp + sizeof(NSMtcphead);
  NSMrequest *reqp = nsmc->req;
  NSMrequest *reqlast = reqp + nsmc->nreq;

  DBS(nsmc,1002);
  
  /* search request */
  msg.req = ntohs(hp->req);
  while (reqp < reqlast) {
    if (reqp->functype != NSMLIB_FNNON && reqp->req == msg.req) break;
    reqp++;
  }
  if (reqp == reqlast) {
    LOG("nsmlib_call unknown req=%04x", msg.req);
    for (reqp = nsmc->req; reqp < reqlast; reqp++) {
      DBG("reqp[%d] %02x %04x", reqp-nsmc->req, reqp->functype, reqp->req);
    }
    return;
  }
  if (! reqp->callback && reqp->functype != NSMLIB_FNSYS) {
    DBG("nsmlib_call no callback for req=%s", reqp->name);
    return;
  }
  DBG("nsmlib_call req=%s", reqp->name);

  DBS(nsmc,1003);
  
  /* fill other msg entries */
  msg.req  = ntohs(hp->req);
  msg.seq  = ntohs(hp->seq);
  msg.node = ntohs(hp->src);
  msg.npar = hp->npar;
  msg.len  = ntohs(hp->len);
  msg.pars[0] = msg.pars[1] = 0; /* to be compatible with NSM1 */
  for (i=0; i < msg.npar; i++) {
    msg.pars[i] = ntohl(*(int32_t *)(recvbuf + i*sizeof(int32_t)));
  }
  msg.datap = msg.len ? recvbuf + msg.npar*sizeof(int32_t) : 0;

  DBS(nsmc,1004);
  
  switch (reqp->functype) {
  case NSMLIB_FNSYS:
    if (! reqp->callback) {
      if (nsmc->reqwait != msg.req) {
	ASSERT("unexpected call %x, waiting for %x", msg.req, nsmc->reqwait);
      }
      nsmc->reqwait = 0;
      DBG("pipewrite pars0=%d", msg.pars[0]);
      DBS(nsmc,1006);
      nsmlib_pipewrite(nsmc, msg.pars[0]);
      DBS(nsmc,1008);
      break;
    }
    /* no break */

  case NSMLIB_FNSTD:
    DBS(nsmc,1010);
    if (! nsmc->hook) {
      DBG("nsmlib_call calling callback without hook");
      DBS(nsmc,1012);
      reqp->callback(&msg, nsmc);
    } else if (! nsmc->hook(&msg, nsmc)) {
      DBG("nsmlib_call calling callback");
      DBS(nsmc,1014);
      reqp->callback(&msg, nsmc);
    } else {
      DBG("nsmlib_call hook error");
    }
    break;

  default:
    DBG("nsmlib_call functype=%d", reqp->functype);
  }
  DBS(nsmc,1016);
}
/* -- nsmlib_handler ------------------------------------------------- */
static void
#ifdef SIGRTMIN
nsmlib_handler(int sig, siginfo_t *info, void *ignored)
#else
nsmlib_handler(int sig)
#endif
{
  sigset_t mask;
  int wait_msec = 1000; /* 1 sec only for the first time */
  
  /* DBG("nsmlib_handler sig=%d", sig); */
  
  /* sigset */
  if (! sig) return; /* this should not be directly called */

  /* in the signal handler mode, it can handle more than one signals */
  while (1) {
    char buf[NSM_TCPMSGSIZ]; /* should not be static */
    NSMcontext *nsmc = nsmlib_selectc(1, wait_msec); /* usesig = 1 */
    wait_msec = 0; /* no wait at the later tries */
    
    if (! nsmc) break;

    /* receive */
    if (nsmlib_recv(nsmc, (NSMtcphead *)buf, 1000) <= 0) { /* 1 sec */
      break;
    }

    /* callback function */
    if (nsmlib_currecursive >= nsmlib_maxrecursive) {
      nsmlib_nskipped++;
      nsmlib_lastskipped = *(NSMtcphead *)buf;
    } else {
      nsmlib_currecursive++;
      DBS(nsmc,1001);
      sigemptyset(&mask);
      sigaddset(&mask, sig);
      sigprocmask(SIG_UNBLOCK, &mask, 0);
      nsmlib_call(nsmc, (NSMtcphead *)buf);
      /*sigprocmask(SIG_BLOCK, &mask, 0);*/
      DBS(nsmc,-1);
      nsmlib_currecursive--;
    }
  };

  if (nsmlib_logfp && nsmlib_loglen && ! nsmlib_currecursive) {
    nsmlib_logflush();
  }
  
  return;
}
/* -- nsmlib_callbackid ---------------------------------------------- */
/*
  Calling with callback == 0 will disable the function,
   except for NSMLIB_FNSYS which has no callback.
   Even with callback == 0, it can be used in the non-signal mode.
  Calling with existing req  will replace the function in   the list.
  Calling with NSMLIB_FNNON  will remove  the function from the list.
  System callback functions are protected from removing/replacing.
 */
int
nsmlib_callbackid(NSMcontext *nsmc, int req, const char *name,
		  NSMcallback_t callback, NSMfunctype functype)
{
  NSMrequest *reqp = nsmc->req;
  NSMrequest *reqlast = reqp + nsmc->nreq;
  NSMrequest *reqfound = 0;
  NSMrequest *reqfree = 0;
  int sigfound = 0;
  
  /* basic checks */
  if (! nsmc || ! nsmc->memp)         return NSMENOINIT;
  if (nsmc->nodeid == NSMSYS_MAX_NOD) return NSMEPERM; /* anonymous */

  /* search for existing and free-space */
  while (reqp < reqlast) {
    if (! reqfree && reqp->functype == NSMLIB_FNNON) {
      reqfree = reqp; /* first free place, but still need to search for req */
    }
    if (reqp->req == req) {
      reqfree = reqfound = reqp;
      if (sigfound) break;
    } else if (reqp->callback) {
      sigfound = 1;
      if (reqfound) break;
    }
    reqp++;
  }

  /* system function check */
  if (nsmc->initsig_done   && functype == NSMLIB_FNSYS) return NSMEINVFUNC;
  if (! nsmc->initsig_done && functype != NSMLIB_FNSYS) return NSMEINVFUNC;
  if (reqfound  &&  reqfound->functype == NSMLIB_FNSYS) return NSMEINVFUNC;
  
  /* attach to the end if not found */
  if (! reqfree) {
    if (nsmc->nreq >= NSMLIB_MAX_CALLBACK) return NSMEMAXFUNC;
    reqfree = reqlast;
    nsmc->nreq++;
  }

  /* update request table */
  reqfree->req      = req;
  reqfree->functype = functype;
  reqfree->callback = callback;
  if (name) {
    strcpy(reqfree->name, name);
  } else {
    sprintf(reqfree->name, "#%04x", req);
  }
  return 0;
} 
/* -- nsmlib_callback ------------------------------------------------ */
int
nsmlib_callback(NSMcontext *nsmc, const char *name, 
		NSMcallback_t callback, NSMfunctype functype)
{
  NSMsys *sysp = nsmc->sysp;
  int hash = nsmlib_hash(sysp, sysp->reqhash, NSMSYS_MAX_HASH, name, 0);
  NSMreq *reqp = (NSMreq *)MEMPTR(sysp, ntohl(sysp->reqhash[hash]));

  if (hash < 0) {
    nsmc->errc = NSMEUNEXPECTED;
    sprintf(nsmc->errs, "hash error for %s", name);
    return -1;
  }

  if (strcmp(reqp->name, name)) {
    nsmc->errc = NSMEUNEXPECTED;
    sprintf(nsmc->errs, "wrong hash? name %s %s\n", reqp->name, name);
    return -1;
  }
  return nsmlib_callbackid(nsmc, ntohs(reqp->code), name,
			   callback, functype);
}
/* -- nsmlib_usesig -------------------------------------------------- */
void
nsmlib_usesig(NSMcontext *nsmc, int usesig)
{
  if (nsmc) nsmc->usesig = usesig;
}
/* -- nsmlib_delclient ----------------------------------------------- */
void
nsmlib_delclient(NSMmsg *msg, NSMcontext *nsmc)
{
  LOG("killed by nsmd2");
  exit(1);
}
/* -- nsmlib_initsig ------------------------------------------------- */
int
nsmlib_initsig(NSMcontext *nsmc)
{
  struct sigaction action;
  int pipes[2];
#ifdef SIGRTMIN
  void nsmlib_handler(int signo, siginfo_t *info, void *ignored);
  nsmlib_sig = SIGRTMIN;
#define NSMLIB_SETHANDLER(s,h) (s).sa_sigaction=(h); (s).sa_flags = SA_SIGINFO|SA_RESTART
#else
  void nsmlib_handler(int);
  nsmlib_sig = SIGUSR1;
#define NSMLIB_SETHANDLER(a,h) (a).sa_sighandler=(h)
#endif

  sigemptyset(&nsmlib_sigmask);
  sigaddset(&nsmlib_sigmask, nsmlib_sig);

  if (! nsmlib_checkpoints) {
    nsmlib_currecursive = 0;
    nsmlib_maxrecursive = NSMLIB_MAXRECURSIVE;
    nsmlib_nskipped = 0;
    memset(&nsmlib_lastskipped, 0, sizeof(nsmlib_lastskipped));
    nsmlib_checkpoints = malloc(sizeof(int) * nsmlib_maxrecursive);
    if (! nsmlib_checkpoints) {
      ASSERT("can't alloc %d bytes", sizeof(int) * nsmlib_maxrecursive);
    }
    memset(nsmlib_checkpoints, 0, sizeof(int) * nsmlib_maxrecursive);
  }

  /* create pipes */
  if (pipe(pipes) < 0) return NSMENOPIPE;
  nsmc->pipe_rd = pipes[0];
  nsmc->pipe_wr = pipes[1];

  /* setup callback handlers */
  nsmlib_callbackid(nsmc, NSMCMD_NEWCLIENT, "cmd_newclient", 0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_ALLOCMEM,  "cmd_allocmem",  0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_OPENMEM,   "cmd_openmem",   0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_FLUSHMEM,  "cmd_flushmem",  0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_NEWREQ,    "cmd_newreq",    0, NSMLIB_FNSYS);
  /* implement delclient as FNSTD */
  nsmlib_callbackid(nsmc, NSMCMD_DELCLIENT, "cmd_delclient",
		    nsmlib_delclient, NSMLIB_FNSYS);
  /* ... and many more to follow ----> */
  
  /* setup signals */
  memset(&action, 0, sizeof(action));
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, nsmlib_sig);
  NSMLIB_SETHANDLER(action, nsmlib_handler);
  /*
    Don't do: action.sa_flags |= SA_RESTART;
    Adding SA_RESTART is not a good idea: a blocking read should
    return with EINTR upon NSM signal, because this NSM signal may be
    the one to clear the blocking condition.
  */
  sigaction(nsmlib_sig, &action, 0);

  nsmc->usesig = -1; /* undecided */
  nsmc->initsig_done = 1;
  return 0;
}
/* -- nsmlib_send ---------------------------------------------------- */
int
nsmlib_send(NSMcontext *nsmc, NSMmsg *msgp)
{
  static char buf[NSM_TCPMSGSIZ];
  NSMtcphead *hp = (NSMtcphead *)buf;
  char *datap  = buf + sizeof(NSMtcphead);
  const char *writep;
  int writelen;
  int writtenlen = 0;
  int err = 0;
  int i;
  int oldsig;

  DBS(nsmc,1100);
  
  if (! nsmc)                         return NSMENOINIT;
  if (nsmc->sock < 0)                 return NSMENOINIT;
  if (nsmc->nodeid == NSMSYS_MAX_NOD) return NSMEPERM; /* anonymous */

  if (msgp->len == 0 && msgp->datap != 0)   return NSMEINVDATA;
  if (msgp->len == 0 && msgp->datap != 0)   return NSMEINVDATA;
  
  DBS(nsmc,1102);
  
  DBG("nsmlib_send nodeid=%d, req=%04x", msgp->node, msgp->req);
  
  hp->src  = htons(nsmc->nodeid);
  hp->dest = htons(msgp->node);
  hp->req  = htons(msgp->req);
  hp->seq  = htons(msgp->seq);
  hp->npar = msgp->npar;
  hp->len  = htons(msgp->len);
  hp->from = 0;
  for (i=0; i<hp->npar; i++) {
    *(int32_t *)datap = htonl(msgp->pars[i]);
    datap += sizeof(int32_t);
  }
  if (msgp->len && msgp->len <= NSM_TCPTHRESHOLD) {
    memcpy(datap, msgp->datap, msgp->len);
    writelen = sizeof(NSMtcphead) + msgp->npar*sizeof(int32_t) + msgp->len;
  } else {
    writelen = sizeof(NSMtcphead) + msgp->npar*sizeof(int32_t);
  }
  writep = buf;

  DBS(nsmc,1104);
  DBG("writep = %x", writep);
  nsmc->errc = 0;

  while (writelen > 0) {
    int ret = nsmlib_select(0, nsmc->sock, 1000); /* 1 sec */
    if (ret <  0) { err = NSMESELECT;  goto nsmlib_send_error; }
    /* socket is not shutdown if nothing was sent and just timed out */
    if (ret == 0 && writtenlen == 0) return NSMETIMEOUT;
    /* wait forever if timed out after something was already written */
    if (ret == 0) {
      nsmc->errc++;
      continue;
    }

    ret = write(nsmc->sock, writep, writelen);
    if (ret < 0 && (errno == EINTR || errno == EAGAIN)) continue;
    if (ret <  0) { err = NSMEWRITE;  goto nsmlib_send_error; }
    if (ret == 0) { err = NSMECLOSED; goto nsmlib_send_error; }

    writtenlen += ret;
    writelen   -= ret;
    writep     += ret;
  }

  if (msgp->len > NSM_TCPTHRESHOLD) {
    writelen = msgp->len;
    writep = msgp->datap;
  }
  
  while (writelen > 0) {
    int ret = nsmlib_select(0, nsmc->sock, 1000); /* 1 sec */
    if (ret <  0) { err = NSMESELECT;  goto nsmlib_send_error; }
    /* wait forever if timed out after something was already written */
    if (ret == 0) {
      nsmc->errc++;
      continue;
    }

    ret = write(nsmc->sock, writep, writelen);
    if (ret < 0 && (errno == EINTR || errno == EAGAIN)) continue;
    if (ret <  0) { err = NSMEWRITE;  goto nsmlib_send_error; }
    if (ret == 0) { err = NSMECLOSED; goto nsmlib_send_error; }

    writtenlen += ret;
    writelen   -= ret;
    writep     += ret;
  }
  DBS(nsmc,1106);
  return 0;
  
 nsmlib_send_error:
  /* errno of select/write will be overwritten by that of shutdown */
  nsmc->errc = errno;
  DBS(nsmc,1199);
  shutdown(nsmc->sock, 2);
  nsmc->sock = -1;
  return err; /* nsmc->errc or nsmlib_errc is set in the caller */
}
/* -- nsmlib_initcli ------------------------------------------------- */
static int
nsmlib_initcli(NSMcontext *nsmc, const char *nodename)
{
  NSMmsg msg;
  int ret;

  /* request setup */
  memset(&msg, 0, sizeof(msg));
  msg.node = -1;
  msg.pars[0] = geteuid();
  msg.pars[1] = getpid();
  msg.npar    = 2;
  msg.datap   = nodename;
  if (nodename) msg.len = strlen(nodename) + 1;
#ifdef SIGRTMIN
  msg.req = NSMCMD_NEWCLIENT;
#else
  msg.req = NSMCMD_NEWCLIENTOB; /* obsolete */
#endif

  /* send request */
  nsmc->reqwait = msg.req;
  if ((ret = nsmlib_send(nsmc, &msg)) < 0) {
    nsmc->reqwait = 0;
    return nsmlib_errc = ret;
  }

  /* receive request */
  ret = nsmlib_recvpar(nsmc);
  DBG("initcli: recvpar=%d", ret);
  if (ret < 0) {
    switch (ret) {
    case NSMENODEEXIST:
      sprintf(nsmlib_errs, "node %s already exists", nodename);
      break;
    case NSMEFULNODE:
      sprintf(nsmlib_errs, "no more place for node %s", nodename);
      break;
    default:
      /* errs just in case when errc is unknown */
      sprintf(nsmlib_errs, "initcli: recvpar error=%d", ret);
    }
    return nsmlib_errc = ret;
  }
  if (ret > NSMSYS_MAX_NOD) {
    sprintf(nsmlib_errs, "invalid nodeid=%d returned from nsmd2", ret);
    return nsmlib_errc = NSMEUNEXPECTED;
  }
  nsmc->nodeid = ret;
  return 0;
}
/* -- nsmlib_sendreqid ----------------------------------------------- */
int
nsmlib_sendreqid(NSMcontext *nsmc,
		 const char *node, uint16_t req, uint npar, int *pars,
		 int len, const char *datap)
{
  NSMmsg msg;
  int i;
  int nodeid = nsmlib_nodeid(nsmc, node);

  if (nodeid < 0) return -1;

  msg.req = req;
  msg.seq = nsmc->seq++;
  msg.node = (uint16_t)nodeid;
  msg.npar = npar > 256 ? 0 : npar;
  for (i=0; i<msg.npar; i++) msg.pars[i] = pars[i];
  msg.len = len;
  msg.datap = datap;
  return (nsmc->errc = nsmlib_send(nsmc, &msg));
}
/* -- nsmlib_reqid --------------------------------------------------- */
int
nsmlib_reqid(NSMcontext *nsmc, const char *reqname)
{
  NSMsys *sysp = nsmc->sysp;
  int hash = nsmlib_hash(sysp, sysp->reqhash, NSMSYS_MAX_HASH, reqname, 0);
  NSMreq *reqp = (NSMreq *)MEMPTR(sysp, ntohl(sysp->reqhash[hash]));

  if (hash < 0) { printf("no hash for %s\n", reqname); return -1; }

  if (strcmp(reqp->name, reqname)) {
    printf("wrong hash? name %s %s\n", reqp->name, reqname);
    return -1;
  }
  return ntohs(reqp->code);
}
/* -- nsmlib_reqname ------------------------------------------------- */
const char *
nsmlib_reqname(NSMcontext *nsmc, int reqid)
{
  NSMsys *sysp = nsmc->sysp;
  reqid -= NSMREQ_FIRST;
  if (reqid < 0 || reqid >= NSMSYS_MAX_REQ) return "(invalid)";
  if (! sysp->req[reqid].name[0]) return "(undef)";
  return sysp->req[reqid].name;
}
/* -- nsmlib_sendreq ------------------------------------------------- */
int
nsmlib_sendreq(NSMcontext *nsmc, const char *node, const char *req,
	       uint npar, int *pars, int len, const char *datap)
{
  int reqid = nsmlib_reqid(nsmc, req);
  if (reqid < 0) { printf("no reqid for %s\n", req); return -1; }
  return nsmlib_sendreqid(nsmc, node, reqid, npar, pars, len, datap);
}
/* -- nsmlib_register_request ---------------------------------------- */
int
nsmlib_register_request(NSMcontext *nsmc, const char *name)
{
  NSMmsg msg;
  int ret;
  int reqid;
  memset(&msg, 0, sizeof(msg));
  msg.req   = NSMCMD_NEWREQ;
  msg.node  = -1;
  msg.len   = strlen(name) + 1;
  msg.datap = name;
  nsmc->reqwait = msg.req;

  /* send request */
  DBG("register_request: before send");
  nsmc->errc = nsmlib_send(nsmc, &msg);
  DBG("register_request: after send");
  if (nsmc->errc < 0) { nsmc->reqwait = 0; return nsmc->errc; }

  /* receive request */
  DBG("register_request: before recvpar");
  ret = nsmlib_recvpar(nsmc);
  DBG("register_request: after recvpar");
  if (ret < 0) return ret;
  return 0;
}
/* -- nsmlib_readmem ------------------------------------------------- */
int
nsmlib_readmem(NSMcontext *nsmc, void *buf,
	       const char *datname, const char *fmtname, int revision)
{
  char fmtstr[256];
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;
  NSMmsg msg;
  int ret;
  int datid;
  NSMdat *datp;
  int newrevision = -1;
  
  if (! fmtname) fmtname = datname;
  
  if (revision <= 0 && revision != -1) {
    sprintf(nsmc->errs, "invalid revision %d for data %s", revision, datname);
    return nsmc->errc = NSMEINVPAR;
  }
  if (! (nsmlib_parsefile(fmtname, revision, nsmlib_incpath, fmtstr,
                          &newrevision))) {
    int errcode;
    const char *errstr = nsmlib_parseerr(&errcode);
    sprintf(nsmc->errs, "cannot read data %s, %s", datname, errstr);
    return nsmc->errc = NSMEPARSE;
  }
  if (revision == -1) {
    revision = newrevision;
  }

  /* linear search, to be replaced with a hash version */
  for (datid = 0; datid < NSMSYS_MAX_DAT; datid++) {
    datp = sysp->dat + datid;
    if (strcmp(datp->dtnam, datname) == 0) break;
  }
  if (datid == NSMSYS_MAX_DAT) {
    sprintf(nsmc->errs, "data %s not found", datname);
    return nsmc->errc = NSMENOMEM;
  }
  if (strcmp(datp->dtfmt, fmtstr) != 0) {
    sprintf(nsmc->errs, "data %s format mismatch %s %s",
	    datname, fmtstr, datp->dtfmt);
    return nsmc->errc = NSMEBADFMT;
  }
  if (ntohs(datp->dtrev) != revision) {
    sprintf(nsmc->errs, "data %s revision mismatch %d %d",
	   datname, revision, datp->dtrev);
    return nsmc->errc = NSMEBADREV;
  }

  memcpy((char *)buf, 
	 (char *)memp + ntohl(datp->dtpos), ntohs(datp->dtsiz));

  return 0;
}
/* -- nsmlib_statmem ------------------------------------------------ */
/*
  return value: size of data if > 0
  fmtbuf is set with name of struct
*/
int
nsmlib_statmem(NSMcontext *nsmc,
               const char *datname, char *fmtbuf, int bufsiz)
{
  NSMsys *sysp = nsmc->sysp;
  NSMdat *datp;
  int datid;
  int dtfmtlen;
  
  if (! fmtbuf) return 0;
    
  /* linear search, to be replaced with a hash version */
  for (datid = 0; datid < NSMSYS_MAX_DAT; datid++) {
    datp = sysp->dat + datid;
    if (strcmp(datp->dtnam, datname) == 0) break;
  }
  if (datid == NSMSYS_MAX_DAT) {
    sprintf(nsmc->errs, "data %s not found", datname);
    nsmc->errc = NSMENOMEM;
    return -1;
  }

  dtfmtlen = strlen(datp->dtfmt);
  *fmtbuf = 0;
  if (datp->dtfmt[dtfmtlen + 1]) {
    int dtstrlen = strlen(datp->dtfmt + dtfmtlen + 1);
    if (dtstrlen < bufsiz) {
      strcpy(fmtbuf, datp->dtfmt + dtfmtlen + 1);
    }
  }
  return (int)ntohs(datp->dtsiz);
}
/* -- nsmlib_openmem ------------------------------------------------- */
void *
nsmlib_openmem(NSMcontext *nsmc,
	       const char *datname, const char *fmtname, int revision)
{
  char fmtstr[256];
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;
  NSMmsg msg;
  int ret;
  int datid;
  NSMdat *datp;
  int newrevision = -1;
  
  if (! fmtname) fmtname = datname;
  
  if (revision <= 0 && revision != -1) {
    sprintf(nsmc->errs, "invalid revision %d for data %s", revision, datname);
    nsmc->errc = NSMEINVPAR;
    return 0;
  }
  if (! (nsmlib_parsefile(fmtname, revision, nsmlib_incpath, fmtstr,
                          &newrevision))) {
    int errcode;
    const char *errstr = nsmlib_parseerr(&errcode);
    sprintf(nsmc->errs, "cannot open data %s, %s", datname, errstr);
    nsmc->errc = NSMEPARSE;
    return 0;
  }
  if (revision == -1) {
    revision = newrevision;
  }

  /* linear search, to be replaced with a hash version */
  for (datid = 0; datid < NSMSYS_MAX_DAT; datid++) {
    datp = sysp->dat + datid;
    if (strcmp(datp->dtnam, datname) == 0) break;
  }
  if (datid == NSMSYS_MAX_DAT) {
    sprintf(nsmc->errs, "data %s not found", datname);
    nsmc->errc = NSMENOMEM;
    return 0;
  }
  if (strcmp(datp->dtfmt, fmtstr) != 0) {
    sprintf(nsmc->errs, "data %s format mismatch %s %s",
	    datname, fmtstr, datp->dtfmt);
    nsmc->errc = NSMEBADFMT;
    return 0;
  }
  if (ntohs(datp->dtrev) != revision) {
    sprintf(nsmc->errs, "data %s revision mismatch %d %d\n",
	   datname, revision, datp->dtrev);
    nsmc->errc = NSMEBADREV;
    return 0;
  }

  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_OPENMEM;
  msg.node = -1;
  msg.npar = 1;
  msg.pars[0] = datid;
  nsmc->reqwait = msg.req;
  if ((nsmc->errc = nsmlib_send(nsmc, &msg)) < 0) {
    nsmc->reqwait = 0;
    return 0;
  }
  if ((ret = nsmlib_recvpar(nsmc)) < 0) {
    return 0;
  }
  if (ret != datp - sysp->dat) {
    sprintf(nsmc->errs, "datid mismatch %d %d", ret, (int)(datp - sysp->dat));
    nsmc->errc = NSMEDATID;
    return 0;
  }
  return (char *)memp + ntohl(datp->dtpos);
}
/* -- nsmlib_flushmem ------------------------------------------------ */
int
nsmlib_flushmem(NSMcontext *nsmc, const void *ptr, int psiz)
{
  NSMmsg msg;
  NSMsys *sysp;
  size_t ppos;
  size_t dpos = 0;
  size_t dsiz = 0;
  int nnext;
  int n = 0; /* to check inf-loop */
  int ret;
  
  if (! nsmc || nsmc->nodeid < 0 || nsmc->nodeid >= NSMSYS_MAX_NOD) {
    return NSMENOINIT;
  }
  
  if (! (sysp = nsmc->sysp) || ! nsmc->memp) {
    return nsmc->errc = NSMENOINIT;
  }
  
  ppos = MEMPOS(nsmc->memp, ptr);
  if (psiz < 0 || ppos < 0 || ppos >= NSM2_MEMSIZ || ppos+psiz > NSM2_MEMSIZ) {
    DBG("flushmem(invptr): psiz=%d ppos=%d", (int)psiz, (int)ppos);
    return nsmc->errc = NSMEINVPTR;
  }

  /* linked list search (only through owned data) */
  nnext = (int16_t)ntohs(sysp->nod[nsmc->nodeid].noddat);
  while (nnext >= 0 && nnext < NSMSYS_MAX_DAT) {
    NSMdat *datp = sysp->dat + nnext;
    dpos = (int32_t)ntohl(datp->dtpos);
    dsiz = (int16_t)ntohs(datp->dtsiz);
    DBG("flushmem: ppos=%d dpos=%d ppos+psiz=%d dpos+dsiz=%d",
        (int)ppos, (int)dpos, (int)(ppos+psiz), (int)(dpos+dsiz));
    if (ppos == dpos && psiz == 0) psiz = dsiz;
    if (ppos >= dpos && ppos+psiz <= dpos+dsiz) break;
    
    nnext = (int16_t)ntohs(datp->nnext);
    if (++n > NSMSYS_MAX_DAT) nnext = -1; /* avoid inf-loop by broken nnext */
  }
  if (nnext < 0 || nnext >= NSMSYS_MAX_DAT) {
    sprintf(nsmc->errs, "data not found for pos=%d siz=%d",
            (int)ppos, (int)psiz);
    return nsmc->errc = NSMENOMEM; /* end of linked list or error */
  }
  
  /* send it */
  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_FLUSHMEM;
  msg.node = -1; /* no particular destination */
  msg.npar = 2;
  msg.pars[0] = ppos;
  msg.pars[1] = psiz;
  DBG("flushmem: psiz=%d ppos=%d", (int)psiz, (int)ppos);
  nsmc->reqwait = msg.req;
  nsmc->errc = nsmlib_send(nsmc, &msg);
  if (nsmc->errc < 0) {
    nsmc->reqwait = 0;
    return 0;
  }

  /* receive response */
  ret = nsmlib_recvpar(nsmc);
  nsmc->reqwait = 0;
  return ret;
}
/* -- nsmlib_allocmem ------------------------------------------------ */
void *
nsmlib_allocmem(NSMcontext *nsmc, const char *datname, const char *fmtname,
		int revision, float cycle)
{
  NSMmsg msg;
  int ret;
  int reqid;
  char fmtstr[256];
  char *p;
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;
  int newrevision = -1;

  DBG("allocmem 1");
  
  if (! fmtname) fmtname = datname;
  if (! nsmc) { nsmlib_errc = NSMENOINIT; return 0; }
  if (! datname) { nsmc->errc = NSMEINVPAR; return 0; }
  if (revision <= 0) { nsmc->errc = NSMEINVPAR; return 0; }
  if (strlen(datname) > NSMSYS_DNAM_SIZ) { nsmc->errc = NSMEINVPAR; return 0; }
  
  if (! (nsmlib_parsefile(fmtname, revision, nsmlib_incpath, fmtstr,
                          &newrevision))) {
    int errcode;
    const char *errstr = nsmlib_parseerr(&errcode);
    sprintf(nsmc->errs, "cannot allocate data %s, %s", datname, errstr);
    nsmc->errc = NSMEPARSE;
    return 0;
  }
  if (revision != newrevision) {
    sprintf(nsmc->errs, "data revision mismatch for %s, %d != %d",
            datname, revision, newrevision);
    nsmc->errc = NSMEPARSE;
    return 0;
  }
  if (strlen(fmtstr) + strlen(fmtname) + 1 > NSMSYS_DFMT_SIZ) {
    nsmc->errc = NSMEINVPAR;
    return 0;
  }
  if (! (p = malloc(strlen(datname) + strlen(fmtstr) + strlen(fmtname) + 3))) {
    nsmc->errc = NSMEALLOC;
    return 0;
  }

  sprintf(p, "%s %s %s", datname, fmtstr, fmtname);

  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_ALLOCMEM;
  msg.node = -1;
  msg.npar = 2;
  msg.pars[0] = (cycle != 0) ? (int)(cycle * 100) : 500; /* in 10ms unit */
  msg.pars[1] = revision;
  msg.len = strlen(p) + 1;
  msg.datap = p;
  DBG("datap=%s\n", p);
  nsmc->reqwait = msg.req;
  nsmc->errc = nsmlib_send(nsmc, &msg);
  free(p);
  if (nsmc->errc < 0) {
    nsmc->reqwait = 0;
    return 0;
  }
  DBG("allocmem 2");
  ret = nsmlib_recvpar(nsmc);
  DBG("allocmem 3");
  nsmc->reqwait = 0;
  if (ret < 0) return 0;
  DBG("allocmem: ret=%d dtpos=%d\n", ret, ntohl(sysp->dat[ret].dtpos));
  p = (char *)memp + ntohl(sysp->dat[ret].dtpos);
  return p;
}
/* -- nsmlib_init ------------------------------------------------------- */
/*    node is anonymous when nodename = 0                                 */
/*                                                                        */
/*    since nsm-1931 IP address for TCP connection is taken from the      */
/*    shared memory and there is no use for the "host" variable nor       */
/*    NSM2_HOST environment variable.                                     */
/*                                                                        */
/* ---------------------------------------------------------------------- */
NSMcontext *
nsmlib_init(const char *nodename, const char *unused, int port, int shmkey)
{
  NSMcontext *nsmc;
  int ret = 0;
  int i;

  if (! nsmlib_logfp) nsmlib_logfp = stdout;

  /* if (! nodename || strlen(nodename) > NSMSYS_NAME_SIZ) ret = -1; */
  if (nodename) {
    if (strlen(nodename) > NSMSYS_NAME_SIZ) ret = -1;
    for (i=0; ret == 0 && nodename[i]; i++) {
      if (! isalnum(nodename[i]) && nodename[i] != '_') ret = -1;
    }
  }
  if (ret < 0) {
    nsmlib_errc = NSMENODENAME;
    return 0;
  }
  
  if (! (nsmc = malloc(sizeof(*nsmc)))) {
    nsmlib_errc = NSMEALLOC;
    return 0;
  }

  /* -- set up nsm context -- */
  memset(nsmc, 0, sizeof(*nsmc));
  strcpy(nsmc->nodename, nodename ? nodename : "(anonymous)");
  nsmc->sock = -1;

  /* -- environment variables -- */
  if (! port)   port   = nsmlib_atoi(getenv(NSMENV_PORT), NSM2_PORT);
  
  /* -- shared memory initialization -- */
  if (ret == 0) ret = nsmlib_initshm(nsmc, shmkey, port);

  /* -- network initialization -- */
  if (ret == 0) ret = nsmlib_initnet(nsmc, 0, port);

  /* -- signal handler initialization -- */
  if (ret == 0) ret = nsmlib_initsig(nsmc);

  /* -- client initialization -- */
  if (ret == 0) ret = nsmlib_initcli(nsmc, nodename);

  /* -- nsm context -- */
  if (ret == 0) {
    nsmc->next = nsmlib_context;
    nsmlib_context = nsmc;    /* -- insert at the top -- */
  } else {
    nsmlib_errc = ret;
    nsmlib_port = port;
    free(nsmc);
    nsmc = 0;
  }

  return nsmc;
}
/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
