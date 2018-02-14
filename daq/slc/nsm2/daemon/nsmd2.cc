// ----------------------------------------------------------------------
// -- nsmd2.cc
//
// - mastership
//   master may change in
//   - init          (to -1)
//   - sch_initbcast (to myself, when no other master is found)
//   - do_ackdaemon  (from master as a reply for a CMD_NEWDAEMON)
//   - do_ready      (from a new daemon, if it is a higher priority one)
//   - do_newmaster  (from old-master, when a new master is set up)
//   - destroyconn   (to -1, when master disappeared)
// ----------------------------------------------------------------------
// revisions
//  20130303  1910 hinamatsuri alpha version
//  20131025  ---- work resuming
//  20131028  ---- suppress recv message
//  20131028  1911 assert no conid case (*1)
//  20131217  1912 more fix about -1 (and major signal fix in corelib)
//  20131217  1913 dtpos fix (don't use ntohs!)
//  20131218  1914 new protocol version, merged with Konno branch
//  20131219  1915 uid/gid for MEM shm
//  20131222  1916 printlog infinite loop fix
//  20131230  1918 argv[0] changed to lower case
//  20140104  1919 disid fix, stdint
//  20140105  1920 nsminfo2
//  20140106  1921 destroyconn fix
//  20140107  1922 fix when master/daemon killed together
//  20140107  1923 priority fix in do_ready
//  20140117  1924 check error before going background, many crucial bug
//                 fixes on nsmd master/deputy switching
//  20140124  1925 fix for anonymous client
//  20140304  1926 nested dtfmt
//  20140305  1927 freeq comparison and timing fix
//  20140305  1928 more freeq checks, longused=1 for debug
//  20140305  1929 longused=0 again
//  20140516  1930 destroycon / delcli fix
//  20140902  1934 static bsizbuf pollution fix, broken tcprecv debug
//  20140902  1935 memset fix
//  20140903  1936 debug message fix
//  20140917  1938 newclient error return fix / shm cleanup fix
//  20140921  1940 flushmem, less DBG messages
//  20140922  1942 nodtim fix
//  20150520  1943 destroyconn fix [for anonymous and for sys.ready]
//  20150521  1944 new protocol version, master recoonect fix
//  20160420  1946 suppress debug output
//  20170613  1947 protect against bad USRCPYMEM from misconnected nodes
//  20170927  1948 touchsys pos fix, log cleanup for send/recv
//  20170929  1949 ackdaemon fix to avoid reconnecting to same M/D
//  20171002  1950 ackdaemon fix to avoid connection to multiple D
//  20171002  1951 destroyconn to non M/D allowed, avoid duplicate free M/D
//  20171002  1952 shortbufp/longbufp/bsizbuf to be modified when ncon changes
//  20180111  1953 debug orphan node
//  20180118  1954 conid fix for master's client, suppress WARN(no conid)
//  20180124  1958 logfp fix for -b, smarter recv message

#define NSM_DAEMON_VERSION   1958
// ----------------------------------------------------------------------

// -- include files -----------------------------------------------------
// ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdint.h>
#include <pwd.h>
#include <grp.h>

#include "nsm2.h"
#include "nsmsys2.h"

// -- useful types ------------------------------------------------------
// ----------------------------------------------------------------------
// typedef listcharp_t  std::list<char *>;
// typedef listcharp_it charplist_t::iterator;

// -- local constants ---------------------------------------------------
// ----------------------------------------------------------------------
#define MAX_ARGS  32

#define NSMD_UDPSENDSIZ (1500-20-8) // one MTU (no fragment allowed in VxW)
#define NSMD_TCPDATASIZ ((1500-20-20)*2 - sizeof(NSMdmsg)) // 2MTU for syscpy
#define NSMD_TCPSENDSIZ     4096 // for other user requests
#define NSMD_UDPBUFSIZ      4096 // socket option
#define NSMD_TCPBUFSIZ      4096 // socket option
#define NSMD_MSGBUFSIZ   (1024*1024) // socket option

// connection status
#define NSMDC_JC  6    // just connected, not ready
#define NSMDC_WS  7    // write select wait
#define NSMDC_OK  0    // OK    : tcpaccept, newconn, tcprecv, reconnect
#define NSMDC_WT  1    // waiting for any reply     : sch_ping
#define NSMDC_NA  2    // not available             : tcpsend / sch_ping
#define NSMDC_WA  3    // waiting for next accept   : reconnect
#define NSMDC_WC  4    // waiting for next connect  : (not used)
#define NSMDC_RM  5    // removed                   : destroyconn / tcpsend


// error codes to be delivered to nsmd_destroy
#define NSMD_EOK      (0)
#define NSMD_ESYSERR  (-1)
#define NSMD_EUNEXPCT (-2)
#define NSMD_EINUSE   (-3)
#define NSMD_ESHMGET  (-4)
#define NSMD_ESHMPERM (-5)
#define NSMD_ENONETIF (-6)
#define NSMD_ETIME    (-7)  // time is not adjusted

// master finding time parameters
#define NSMD_INITCOUNT_FIRST  4 // shortend from 10 for debug
#define NSMD_INITCOUNT_SECOND 8 // shortend from 15 for debug
#define NSMD_INITCOUNT_DELTA  2
#define NSMD_CONNECT_TIMEOUT  5 // this must be smaller than INITCOUNT

// -- global variables --------------------------------------------------
// ----------------------------------------------------------------------
uint16_t    nsmd_port   = NSM2_PORT;
int         nsmd_shmkey = -1; // == nsmd_port if -1
int         nsmd_debug  = 0;
int         nsmd_priority = 0;
uint32_t    nsmd_myip   = 0;  // network byte order
SOCKAD_IN   nsmd_sockad;
char        nsmd_host[1024];
const char* nsmd_logdir = ".";
FILE*       nsmd_logfp = stdout;
#define NSMD_DBGAFTER 4
#define NSMD_DBGBEFORE 8
char        nsmd_dbgbuf[NSMD_DBGBEFORE][256];
int         nsmd_dbgcnt = 0;
int         nsmd_shmsysid = -1;
int         nsmd_shmmemid = -1;
const char* nsmd_usrnam = 0; // default is "nsm", set in nsmd_main
const char* nsmd_grpnam = 0; // default is "nsm", set in nsmd_main
NSMsys*     nsmd_sysp = 0;
NSMmem*     nsmd_memp = 0;

static int nsmd_init_count = NSMD_INITCOUNT_FIRST;

/*
  Default is to store twice the largest packet, but:

  - Default Linux maximum (/proc/sys/net/core/wmem_default) is 131071
    and will be truncated by setsockopt without an error.
  - Default Solaris maximum (tcp_max_buf) 1048576 (0x100000) and
    should be okay.
  - To cope with a system which makes a system call error, this parameter
    is a global variable and will be adjusted by a command parameter.
 */

int nsmd_tcpsocksiz = NSM_TCPMSGSIZ * 2;

// TCP write queue
static NSMDtcpq* nsmd_tcpqfirst = 0;
static NSMDtcpq* nsmd_tcpqlast = 0;

// buffer to store partially received data
const int MAX_CONI = NSMSYS_MAX_CON - NSMCON_OUT;
const int NO_CONI = -1;
static char* recv_shortbufp[MAX_CONI];
static char* recv_longbufp[MAX_CONI];
static int32_t recv_bsizbuf[MAX_CONI];

// -- command list ------------------------------------------------------
// ----------------------------------------------------------------------
typedef int (*NSMschfunc_t)(int16_t, int32_t);
typedef struct { const char* name; NSMschfunc_t func; } NSMschtbl_t;

#define NSMSCH_INITBCAST 1
#define NSMSCH_INITMEM   2
int nsmd_sch_initbcast(int16_t, int32_t);
int nsmd_sch_initmem(int16_t, int32_t) {}

static NSMschtbl_t nsmd_schlist[] = {
  { "(none)", 0 },
  { "initbcast", nsmd_sch_initbcast }, // NSMSCH_INITBCAST
  { "initmem",   nsmd_sch_initmem },   // NSMSCH_INITMEM
  { 0, 0 }
};

typedef void (*NSMcmdfunc_t)(NSMcon&, NSMdmsg&);
typedef struct {
  int          req;
  int          printable;
  const char*  name;
  NSMcmdfunc_t func;
} NSMcmdtbl_t;

void nsmd_do_newclient(NSMcon&, NSMdmsg&);

void nsmd_do_newmaster(NSMcon&, NSMdmsg&);
void nsmd_do_delclient(NSMcon&, NSMdmsg&);
void nsmd_do_allocmem(NSMcon&, NSMdmsg&);
void nsmd_do_freemem(NSMcon&, NSMdmsg&) {}
void nsmd_do_flushmem(NSMcon&, NSMdmsg&);
void nsmd_do_reqcpymem(NSMcon&, NSMdmsg&);
void nsmd_do_syscpymem(NSMcon&, NSMdmsg&);
void nsmd_do_openmem(NSMcon&, NSMdmsg&);
void nsmd_do_closemem(NSMcon&, NSMdmsg&) {}
void nsmd_do_dumpnode(NSMcon&, NSMdmsg&) {}
void nsmd_do_dumpalloc(NSMcon&, NSMdmsg&) {}
void nsmd_do_dumpdata(NSMcon&, NSMdmsg&) {}
void nsmd_do_dumpconn(NSMcon&, NSMdmsg&) {}
void nsmd_do_dumpinfo(NSMcon&, NSMdmsg&) {}
void nsmd_do_debugflag(NSMcon&, NSMdmsg&) {}
void nsmd_do_killdaemon(NSMcon&, NSMdmsg&);
void nsmd_do_cleanup(NSMcon&, NSMdmsg&) {}
void nsmd_do_touchmem(NSMcon&, NSMdmsg&) {}
void nsmd_do_ping(NSMcon&, NSMdmsg&) {}
void nsmd_do_pong(NSMcon&, NSMdmsg&) {}
void nsmd_do_ready(NSMcon&, NSMdmsg&);
void nsmd_do_nop(NSMcon&, NSMdmsg&) {}
void nsmd_do_newreq(NSMcon&, NSMdmsg&);
void nsmd_do_delreq(NSMcon&, NSMdmsg&);
void nsmd_do_newdaemon(NSMcon&, NSMdmsg&);       // udp
void nsmd_do_ackdaemon(NSMcon&, NSMdmsg&);       // udp
void nsmd_do_usrcpymem(NSMcon&, NSMdmsg&);       // udp


static NSMcmdtbl_t nsmd_cmdtbl[] = {
  // tcp
  { NSMCMD_NEWCLIENT,   1, "NEWCLIENT",   nsmd_do_newclient },
  { NSMCMD_NEWCLIENTOB, 1, "NEWCLIENTOB", nsmd_do_newclient },
  { NSMCMD_NEWMASTER,   0, "NEWMASTER",   nsmd_do_newmaster },
  { NSMCMD_DELCLIENT,   0, "DELCLIENT",   nsmd_do_delclient },
  { NSMCMD_ALLOCMEM,    0, "ALLOCMEM",    nsmd_do_allocmem },
  { NSMCMD_FREEMEM,     0, "FREEMEM",     nsmd_do_freemem },
  { NSMCMD_FLUSHMEM,    0, "FLUSHMEM",    nsmd_do_flushmem },
  { NSMCMD_REQCPYMEM,   0, "REQCPYMEM",   nsmd_do_reqcpymem },
  { NSMCMD_SYSCPYMEM,   0, "SYSCPYMEM",   nsmd_do_syscpymem },
  { NSMCMD_OPENMEM,     0, "OPENMEM",     nsmd_do_openmem },
  { NSMCMD_CLOSEMEM,    0, "CLOSEMEM",    nsmd_do_closemem },
  { NSMCMD_DUMPNODE,    0, "DUMPNODE",    nsmd_do_dumpnode },
  { NSMCMD_DUMPALLOC,   0, "DUMPALLOC",   nsmd_do_dumpalloc },
  { NSMCMD_DUMPDATA,    0, "DUMPDATA",    nsmd_do_dumpdata },
  { NSMCMD_DUMPCONN,    0, "DUMPCONN",    nsmd_do_dumpconn },
  { NSMCMD_DUMPINFO,    0, "DUMPINFO",    nsmd_do_dumpinfo },
  { NSMCMD_DEBUGFLAG,   0, "DEBUGFLAG",   nsmd_do_debugflag },
  { NSMCMD_KILLDAEMON,  0, "KILLDAEMON",  nsmd_do_killdaemon },
  { NSMCMD_CLEANUP,     0, "CLEANUP",     nsmd_do_cleanup },
  { NSMCMD_TOUCHMEM,    0, "TOUCHMEM",    nsmd_do_touchmem },
  { NSMCMD_PING,        0, "PING",        nsmd_do_ping },
  { NSMCMD_PONG,        0, "PONG",        nsmd_do_pong },
  { NSMCMD_READY,       0, "READY",       nsmd_do_ready },
  { NSMCMD_NOP,         0, "NOP",         nsmd_do_nop },
  { NSMCMD_NEWREQ,      1, "NEWREQ",      nsmd_do_newreq },
  { NSMCMD_DELREQ,      0, "DELREQ",      nsmd_do_delreq },
  // udp-broadcast
  { NSMCMD_NEWDAEMON,   0, "NEWDAEMON",   nsmd_do_newdaemon },
  { NSMCMD_ACKDAEMON,   0, "ACKDAEMON",   nsmd_do_ackdaemon },
  { NSMCMD_USRCPYMEM,   0, "USRCPYMEM",   nsmd_do_usrcpymem },
};

// -- useful macros -----------------------------------------------------
// ----------------------------------------------------------------------
#define DBG   nsmd_dbg
#define LOG   nsmd_log
#define WARN  nsmd_warn
#define ERRO  nsmd_error
#define ASRT  nsmd_assert

#define lengthof(a) (sizeof(a)/sizeof((a)[0]))

#define VSNPRINTF(buf, ap, fmt) \
  va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap)
#define VFPRINTF(fp, ap, fmt) \
  va_start(ap, fmt); vfprintf(fp, fmt, ap); va_end(ap)

#define SOCKOPT(s,o,v) setsockopt(s,SOL_SOCKET,o,(char *)&(v),sizeof(v))
#define BIND(s,sap,p) \
  (sap)->sin_port=htons((short)(p)),\
                  (sap)->sin_family=AF_INET,\
                                    bind(sock,(struct sockaddr *)(sap),sizeof(*(sap)))
#define RECVFROM(sock,buf,from,len) \
  recvfrom(sock,(char *)&buf,sizeof(buf),0,(struct sockaddr *)from,len)
#define ADDR_IP(a)    ((uint32_t)((a).sin_addr.s_addr)) // network-byte-order
#define ADDR_EQ(a,b) (ADDR_IP(a) == ADDR_IP(b))

#define ADDR_STR(a)  nsmd_addrstr(a)
#define NODE_STR(a)  nsmd_nodestr(a)
#define CON_ID(c)    (nsmd_sysp ? (&(c) - nsmd_sysp->con) : NSMCON_NON)


/* -- other shorthands -- */
#define NoMaster()      (nsmd_sysp->master == NSMCON_NON)
#define NoDeputy()      (nsmd_sysp->deputy == NSMCON_NON)
#define IamMaster()     (nsmd_sysp->master == NSMCON_TCP)
#define IamDeputy()     (nsmd_sysp->deputy == NSMCON_TCP)
#define ConidIsLocal(i) (AddrConid(i) == nsmd_myip)
#define ConIsLocal(con) (ADDR_IP(con.sockad) == nsmd_myip)
#define ConIsMaster(c)  (nsmd_sysp && CON_ID(c) == nsmd_sysp->master)
#define ConIsDeputy(c)  (nsmd_sysp && CON_ID(c) == nsmd_sysp->deputy)

#define AddrSelf()     AddrConid(NSMCON_TCP)
#define AddrMaster()   AddrConid(nsmd_sysp->master)
#define AddrDeputy()   AddrConid(nsmd_sysp->deputy)
#define AddrConid(i)   (ExistConid(i)?ADDR_IP(nsmd_sysp->con[i].sockad):(-1))

#define ExistConid(i)   (i > NSMCON_NON && (nsmd_sysp && i < nsmd_sysp->ncon))
//#define ExistNode(node) (node >= 0 && (node <= NSMSYS_MAX_NOD) && \
//                         nsmd_sysp && nsmd_sysp->nod[node].nodeid == node)
#define NodeIsValid(i)  (nsmd_sysp && (uint16_t)(i) < NSMSYS_MAX_NOD)
#define ExistMaster()   ExistConid(nsmd_sysp->master)
#define ExistDeputy()   ExistConid(nsmd_sysp->master)

#define SYSPOS(ptr) ((char *)(ptr) - (char *)nsmd_sysp)
#define SYSPTR(pos) (((char *)nsmd_sysp)+pos)
#define MEMPOS(ptr) ((char *)(ptr) - (char *)nsmd_memp)
#define MEMPTR(pos) (((char *)nsmd_memp)+pos)
#define MEMALIGN(a) (((a)+7)&~7)

#define DBGBIT(a) (nsmd_debug&(1<<(a)))

// -- forward declarations ----------------------------------------------
// ----------------------------------------------------------------------
void nsmd_dbg(const char* fmt, ...);
void nsmd_log(const char* fmt, ...);
void nsmd_warn(const char* fmt, ...);
void nsmd_error(const char* fmt, ...);
void nsmd_assert(const char* fmt, ...);

static void nsmd_setup_daemon(NSMcon& con, int exception = NSMCON_NON);
static void nsmd_destroyconn(NSMcon& con, int delcli, const char* bywhom);
static void nsmd_tcpsend(NSMcon& con, NSMdmsg& dmsg,
                         NSMDtcpq* qptr = 0, int beforeafter = 0);
extern "C" int nsmlib_hash(NSMsys* sysp, int32_t* hashtable, int hashmax,
                           const char* key, int create);
extern "C" int nsmlib_hashcode(const char* key, int hashmax);
extern void nsminfo2();

//                   -------------------------
// --                -- low level functions --
//                   -------------------------

// -- time10ms ----------------------------------------------------------
static uint64_t
time10ms()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (uint64_t)tv.tv_sec * 100 + tv.tv_usec / 10000; // in 10msec unit
}
// -- time1ms -----------------------------------------------------------
static uint64_t
time1ms()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000; // in 1msec unit
}
// -- sleep10ms ---------------------------------------------------------
static int
sleep10ms(int t10ms, int wait = 0)
{
  struct timespec req;
  struct timespec rem;
  req.tv_sec  = t10ms / 100;
  req.tv_nsec = (t10ms % 100) * 1000 * 1000;
  while (1) {
    if (nanosleep(&req, &rem) == 0) return 0;
    if (errno != EINTR) ASRT("sleep10ns %d", t10ms);
    if (! wait) return rem.tv_sec * 100 + rem.tv_nsec / (1000 * 1000);
    req = rem;
  }
}
// -- sleep1ms ----------------------------------------------------------
static int
sleep1ms(int t1ms, int wait = 0)
{
  struct timespec req;
  struct timespec rem;
  req.tv_sec  = t1ms / 1000;
  req.tv_nsec = (t1ms % 1000) * 1000 * 1000;
  while (1) {
    if (nanosleep(&req, &rem) == 0) return 0;
    if (errno != EINTR) ASRT("sleep1ms %d", t1ms);
    if (! wait) return rem.tv_sec * 1000 + rem.tv_nsec / (1000 * 1000);
    req = rem;
  }
}
// -- htonll ------------------------------------------------------------
#define ntohll(a) htonll(a)
static uint64_t
htonll(uint64_t h)
{
  static uint16_t n42 = htons(42); /* 42 is the answer --- Douglas Adams */
  if (n42 == 42) {
    return h;
  } else {
    uint64_t n;
    uint32_t* hp = (uint32_t*)&h;
    uint32_t* np = (uint32_t*)&n;
    np[0] = htonl(hp[1]);
    np[1] = htonl(hp[0]);
    return n;
  }
}
// -- hlltohl2 ----------------------------------------------------------
// to send a uint64_t as two uint32_t words
// ----------------------------------------------------------------------
void
hlltohl2(uint64_t hll, uint32_t& h0, uint32_t& h1)
{
  static uint16_t n42 = htons(42); /* 42 is the answer */
  uint32_t* hp = (uint32_t*)&hll;
  if (n42 == 42) {
    h0 = hp[0];
    h1 = hp[1];
  } else {
    h0 = hp[1];
    h1 = hp[0];
  }
}
// -- hlltohl -----------------------------------------------------------
// to send a uint64_t as two uint32_t words
// ----------------------------------------------------------------------
uint32_t
hlltohl(uint64_t hll, int i)
{
  static uint16_t n42 = htons(42); /* 42 is the answer */
  uint32_t* hp = (uint32_t*)&hll;
  return (n42 == 42) ? hp[i & 1] : hp[1 - (i & 1)];
}
// -- hl2tohll ----------------------------------------------------------
// to receive a uint64_t as two uint32_t words
// ----------------------------------------------------------------------
uint64_t
hl2tohll(uint32_t h0, uint32_t h1)
{
  static int n42 = htons(42); /* 42 is the answer */
  uint64_t h;
  uint32_t* hp = (uint32_t*)&h;
  if (n42 == 42) {
    hp[0] = h0;
    hp[1] = h1;
  } else {
    hp[0] = h1;
    hp[1] = h0;
  }
  return h;
}
// -- nsmd_free ---------------------------------------------------------
// a wrapping function for free
// ----------------------------------------------------------------------
static int     nsmd_alloctimes = 0;
static int64_t nsmd_alloctotal = 0;
static int64_t nsmd_allocalloc = 0;
static char    nsmd_longbuf[NSM_TCPBUFSIZ];
static int     nsmd_longused = 0;

void
nsmd_free(const char* where, const char* p)
{
  //DBG("free: %s p=%08x\n", where, p);

  if (SYSPOS(p) >= 0 && SYSPOS(p) < sizeof(NSMsys)) return;
  if (MEMPOS(p) >= 0 && MEMPOS(p) < sizeof(NSMmem)) return;
  if (p == nsmd_longbuf && nsmd_longused) {
    nsmd_longused = 0;
    return;
  }
  if (p == nsmd_longbuf && ! nsmd_longused) ASRT("nsmd_free: unused longbuf");
  if (p > nsmd_longbuf && p < nsmd_longbuf + NSM_TCPBUFSIZ)
    ASRT("nsmd_free: invalid longbuf usage");

  nsmd_allocalloc -= *(int*)(p - 16);
  free((char*)p - 16);
}
// -- nsmd_malloc -------------------------------------------------------
// a wrapping function for malloc
// ----------------------------------------------------------------------
static void*
nsmd_malloc(const char* where, int size)
{
  char* p;
  if (size <= 0) ASRT("cannot malloc %d bytes for %s", size, where);
  p = (char*)malloc(size + 16);
  nsmd_alloctimes++;
  nsmd_alloctotal += size + 16;
  nsmd_allocalloc += size + 16;
  if (! p) {
    ERRO("malloc(%d) at %s (%d times, total/cur %ld/%ld bytes)",
         size, where, nsmd_alloctimes, nsmd_alloctotal, nsmd_allocalloc);
  }
  *(int*)p = size;
  return p + 16;
}
// -- nsmd_atoi ---------------------------------------------------------
//    a bit more intelligent than atoi
// ----------------------------------------------------------------------
int
nsmd_atoi(const char* a, int def = 0)
{
  if (! a || ! *a) return def;
  if (!(isdigit(*a) || (*a == '-' && isdigit(*(a + 1))))) {
    return def;
  } else {
    return (int)strtol(a, 0, 0);
  }
}
// -- nsmd_addrstr ------------------------------------------------------
// ip : network byte order, after ntohl for 127.0.0.1 => 0x7f000001.
// ----------------------------------------------------------------------
const char*
nsmd_addrstr(int ip)
{
  NSMsys& sys = *nsmd_sysp;
  static char nodestr[16][64];
  static int nodeidx = 0;
  int h = ntohl(ip);

  char* nodestrp = nodestr[nodeidx];
  nodeidx = (nodeidx + 1) % 16;
  sprintf(nodestrp, "%d.%d.%d.%d", (h >> 24) & 255, (h >> 16) & 255, (h >> 8) & 255, h & 255);
  return nodestrp;
}
const char*
nsmd_addrstr(const SOCKAD_IN& sockad)
{
  return nsmd_addrstr(ADDR_IP(sockad));
}
// -- nsmd_nodestr ------------------------------------------------------
// nsmd_nodestr (= NODE_STR)
// returns nodename or ip address
// nodeid : host byte order, for 127.0.0.1 => 0x7f000001.
// ----------------------------------------------------------------------
const char*
nsmd_nodestr(int nodeid)
{
  NSMsys& sys = *nsmd_sysp;
  static char nodestr[16][64];
  static int nodeidx = 0;

  char* nodestrp = nodestr[nodeidx];
  nodeidx = (nodeidx + 1) % 16;

  if (nodeid == -1) {
    strcpy(nodestrp, "()");
  } else if (nodeid == NSMSYS_MAX_NOD) {
    strcpy(nodestrp, "[anonym]");
  } else if (&sys && nodeid == ntohl(ADDR_IP(sys.con[NSMCON_UDP].sockad))) {
    strcpy(nodestrp, "[bcast]");
  } else if (nodeid < 0 || nodeid > NSMSYS_MAX_NOD) {
    return nsmd_addrstr(nodeid);
  } else if (! &sys) {
    strcpy(nodestrp, "[null]"); // unexpected
  } else if (sys.nod[nodeid].name[0]) {
    sprintf(nodestrp, "%-31.31s", sys.nod[nodeid].name); // NSMSYS_NAME_SIZ=32
    char* p = strchr(nodestrp, ' ');
    if (p) *p = 0;
  } else {
    sprintf(nodestrp, "[node=%d]", nodeid);
  }
  return nodestrp;
}
// -- nsmd_destroy_clients ----------------------------------------------
//    called from nsmd_destroy and also when orphan shm is found
// ----------------------------------------------------------------------
void
nsmd_destroy_clients()
{
  NSMsys& sys = *nsmd_sysp;
  if (! nsmd_sysp) return;

  uint64_t now = time10ms();
  uint64_t timeout = now + 500; // +5 sec
  int sig = SIGTERM;
  while (nsmd_sysp && now < timeout) {
    if (now >= timeout) sig = SIGKILL;
    int nkill = 0;
    for (int i = NSMCON_OUT; i < sys.ncon; i++) {
      NSMcon& con = sys.con[i];
      if (con.pid > 0 && kill(con.pid, sig) < 0 && errno == ESRCH) con.pid = 0;
      if (con.pid > 0) nkill++;
    }
    if (nkill == 0) break;
    sleep10ms(10); // 100 ms
    now = time10ms();
  }
}
// -- nsmd_destroy ------------------------------------------------------
//    signal handler to destroy
//    exit_code
//      0 -- normal exit
//      1 -- can't shmget  (code = -1)
// ----------------------------------------------------------------------
void
nsmd_destroy(int code = NSMD_EUNEXPCT)
{
  char str[32];
  int exit_code = 0; // normal exit
  NSMsys& sys = *nsmd_sysp;
  NSMmem& mem = *nsmd_memp;

  switch (code) {
    // internal errors
    case NSMD_EOK:      strcpy(str, ""); break;
    case NSMD_ESYSERR:  strcpy(str, " by syscall");    exit_code = -code; break;
    case NSMD_EINUSE:   strcpy(str, " by adrs-in-use"); exit_code = -code; break;
    case NSMD_EUNEXPCT: strcpy(str, " unexpectedly");  exit_code = -code; break;
    case NSMD_ESHMGET:  strcpy(str, " by shmget");     exit_code = -code; break;
    case NSMD_ESHMPERM: strcpy(str, " by shmperm");    exit_code = -code; break;
    case NSMD_ENONETIF: strcpy(str, " by no-net-if");  exit_code = -code; break;
    case NSMD_ETIME:    strcpy(str, " by wrong-time"); exit_code = -code; break;
    // signals
    case SIGTERM:      strcpy(str, " by SIGTERM"); break;
    case SIGINT:       strcpy(str, " by SIGINT");  break;
    case SIGCHLD:      strcpy(str, " by SIGCHLD"); break;
    case SIGPIPE:      strcpy(str, " by SIGPIPE"); break;
    default:           sprintf(str, " by signal=%d", code);
  }

  if (code > 0) {
    LOG("calling nsminfo2");
    nsminfo2();
  }

  LOG("terminating%s...", str);

  // for nsminfo2 which may be grabbing the screen
  time_t now = time(0);
  if (nsmd_memp) {
    mem.timstart = (time_t) - 1;
    mem.timevent = now;
  }
  if (nsmd_sysp) {
    sys.timstart = (time_t) - 1;
    sys.timevent = now;
  }

  nsmd_destroy_clients();

  if (nsmd_shmsysid >= 0) {
    /* can't we count the number of links? */
    LOG("deleting shm(sys)...");
    int ret = shmctl(nsmd_shmsysid, IPC_RMID, 0);
    nsmd_shmsysid = 0;
  }
  if (nsmd_shmmemid >= 0) {
    /* can't we count the number of links? */
    LOG("deleting shm(mem)...");
    int ret = shmctl(nsmd_shmmemid, IPC_RMID, 0);
    nsmd_shmmemid = 0;
  }
  exit(exit_code);
}
// -- nsmd_brokenpipe ---------------------------------------------------
//    signal handler for SIGPIPE
//    -------------------------------------------------------------------
void
nsmd_brokenpipe(int code)
{
  signal(SIGPIPE, nsmd_brokenpipe);
  WARN("broken pipe");
}
// -- nsmd_reopenlog ----------------------------------------------------
//    log directory
// ----------------------------------------------------------------------
int
nsmd_reopenlog()
{
  static int logid = -1;
  static char logprefix[1024];
  static int  isdirprefix = 0;
  char logfile[1024];
  time_t now = time(0);
  tm* cur = localtime(&now);
  struct stat statbuf;

  if (! logprefix[0]) {
    strcpy(logprefix, nsmd_logdir);
    if (! logprefix[0]) strcpy(logprefix, ".");
    if (logprefix[strlen(logprefix) - 1] == '/') {
      logprefix[strlen(logprefix) - 1] = 0;
    }
    if (stat(logprefix, &statbuf) >= 0 && S_ISDIR(statbuf.st_mode)) {
      sprintf(logprefix + strlen(logprefix),
              "/nsmd-%s-%d", nsmd_host, nsmd_port);
    }
  }

  sprintf(logfile, "%s.%04d%02d%02d.log", logprefix,
          cur->tm_year + 1900, cur->tm_mon + 1, cur->tm_mday);
  if (nsmd_logfp != stdout) {
    if (nsmd_logfp) {
      fprintf(nsmd_logfp, "switching to a new log file %s...\n", logfile);
      fclose(nsmd_logfp);
    }
    if (!(nsmd_logfp = fopen(logfile, "a"))) {
      printf("cannot open logfile %s\n", logfile);
      exit(1);
    }
  }
  return 0;
}
// -- nsmd_print0 / nsmd_print1 -----------------------------------------
//    printf to be called from nsminfo2
// ----------------------------------------------------------------------
void
nsmd_print0(const char* fmt, ...)
{
  va_list ap;
  fputs("* ", nsmd_logfp);
  VFPRINTF(nsmd_logfp, ap, fmt);
}
void
nsmd_print1(const char* fmt, ...)
{
  va_list ap;
  VFPRINTF(nsmd_logfp, ap, fmt);
}
// -- nsmd_flushdbg -----------------------------------------------------
//    flushdbg
// ----------------------------------------------------------------------
void
nsmd_flushdbg()
{
  if (nsmd_dbgcnt > NSMD_DBGAFTER) {
    int ifirst = 0;
    int imax = nsmd_dbgcnt - NSMD_DBGAFTER;
    if (imax > NSMD_DBGBEFORE) {
      fprintf(nsmd_logfp, "(skipping %d lines)\n", imax - NSMD_DBGBEFORE);
      imax = NSMD_DBGBEFORE;
      ifirst = (imax % NSMD_DBGAFTER);
    }

    for (int i = 0; i < imax; i++) {
      const char* p = nsmd_dbgbuf[(ifirst + i) % NSMD_DBGAFTER];
      fputs(p, nsmd_logfp);
      fputc('\n', nsmd_logfp);
    }
  }
  nsmd_dbgcnt = 0;
}
// -- nsmd_logtime ------------------------------------------------------
//    logtime
// ----------------------------------------------------------------------
void
nsmd_logtime(char* buf)
{
  timeval now;
  tm* cur;
  static int lastday = -1;
  extern void nsmd_printlog(const char* prompt, const char* str);

  gettimeofday(&now, 0);
  cur = localtime((time_t*)&now.tv_sec);
  if (lastday != cur->tm_yday) {
    char datebuf[128];
    int dver = NSM_DAEMON_VERSION;
    int pver = NSM_PROTOCOL_VERSION;
    lastday = cur->tm_yday;
    nsmd_flushdbg();
    nsmd_reopenlog();
    nsmd_print1("%s%s\n",
                "----------------------------------",
                "---------------------------------");
    nsmd_print1("%s version %d.%d.%02d protocol %d.%d.%02d\n",
                "NSMD2 - network shared memory daemon",
                dver / 1000, (dver / 100) % 10, dver % 100,
                pver / 1000, (pver / 100) % 10, pver % 100);

    /*
    nsmd_print1("date: %04d.%02d.%02d\n",
    cur->tm_year+1900, cur->tm_mon+1, cur->tm_mday);
    */
    /* nsminfo2(); */
    nsmd_print1("%s%s\n",
                "----------------------------------",
                "---------------------------------");
  }
  sprintf(buf, "%02d:%02d:%02d.%03d ",
          cur->tm_hour, cur->tm_min, cur->tm_sec, (int)now.tv_usec / 1000);
}
// -- nsmd_printlog -----------------------------------------------------
//    printlog
// ----------------------------------------------------------------------
void
nsmd_printlog(const char* prompt, const char* str, int isdbg = 0)
{
  char datebuf[32];

  if (! isdbg) nsmd_flushdbg();

  nsmd_logtime(datebuf);
  const char* p = str;
  while (p && *p) {
    const char* q = strchr(p, '\n');
    fputs(datebuf, nsmd_logfp);
    fputs(prompt, nsmd_logfp);
    if (p != str) fputs("+ ", nsmd_logfp);
    if (q) {
      fwrite(p, q - p + 1, 1, nsmd_logfp);
      p = q + 1;
    } else {
      fputs(p, nsmd_logfp);
      fputc('\n', nsmd_logfp);
      p = 0;
    }
  }
}
// -- nsmd_error --------------------------------------------------------
//    system call error and exit
// ----------------------------------------------------------------------
void
nsmd_error(const char* fmt, ...)
{
  char buf[4096];
  va_list ap;
  VSNPRINTF(buf, ap, fmt);

  snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
           " (%s)", errno ? strerror(errno) : "errno=0");
  errno = 0;

  nsmd_printlog("[ERROR] ", buf);
  nsmd_destroy(NSMD_ESYSERR);
}
// -- nsmd_assert -------------------------------------------------------
//    assert error condition and die
// ----------------------------------------------------------------------
void
nsmd_assert(const char* fmt, ...)
{
  char buf[4096];
  va_list ap;
  VSNPRINTF(buf, ap, fmt);
  nsmd_printlog("[ASSERT] ", buf);
  nsmd_destroy(NSMD_ESYSERR);
}
// -- nsmd_dbg ----------------------------------------------------------
//    debug message in printf style (only during debug, not to be kept)
// ----------------------------------------------------------------------
void
nsmd_dbg(const char* fmt, ...)
{
  char buf[4096];
  va_list ap;
  VSNPRINTF(buf, ap, fmt);
  if (nsmd_dbgcnt < NSMD_DBGAFTER) {
    nsmd_printlog("DBG: ", buf, /* isdbg */ 1);
  } else {
    int i = (nsmd_dbgcnt - NSMD_DBGAFTER) % NSMD_DBGBEFORE;
    char* p = nsmd_dbgbuf[i];
    int siz = sizeof(nsmd_dbgbuf[0]);
    // strlen(p) = 13 ("%02d:%02d:%02d.%03d ")
    nsmd_logtime(p);
    if (nsmd_dbgcnt == 0) {    // maybe changed by nsmd_logtime()
      nsmd_printlog("DBG: ", buf, /* isdbg */ 1);
      return;
    }
    strcpy(p + 13, "DBG: ");
    strncpy(p + 18, buf, siz - 18);
    p[siz - 1] = 0;                  // null-terminate if longer than siz
    if (p = strchr(p, '\n')) * p = 0; // no multiple lines
  }
  nsmd_dbgcnt++;
}
// -- nsmd_log ----------------------------------------------------------
//    usual log message in printf style
// ----------------------------------------------------------------------
void
nsmd_log(const char* fmt, ...)
{
  char buf[4096];
  va_list ap;
  VSNPRINTF(buf, ap, fmt);
  nsmd_printlog("", buf);
}
// -- nsmd_warn ---------------------------------------------------------
//    same as nsmd_log, except [WARNING] is added
// ----------------------------------------------------------------------
void
nsmd_warn(const char* fmt, ...)
{
  char buf[4096];
  va_list ap;
  VSNPRINTF(buf, ap, fmt);
  nsmd_printlog("[WARNING] ", buf);
}
//                   --------------------
// -- nsmd_dumpall ------------------------------------------------------
//    dump everything to log
// ----------------------------------------------------------------------
void
nsmd_dumpall(int signo)
{
  if (signo) {
    signal(signo, nsmd_dumpall);
    LOG("signal %s", signo == SIGUSR1 ? "USR1" : "HUP");
  }
}
// -- nsmd_schedule -----------------------------------------------------
void
nsmd_schedule(int conid, int funcid, int opt, int dt, int doitfirst)
{
  NSMsys& sys = *nsmd_sysp;
  NSMsch* schp;

  DBG("nsmd_schedule conid=%d funcid=%d", conid, funcid);

  for (schp = sys.sch; schp < sys.sch + sys.nsch; schp++) {
    if (schp->conid == conid && schp->funcid == funcid) break;
  }
  if (schp == sys.sch + sys.nsch) sys.nsch++;
  if (schp >= sys.sch + NSMSYS_MAX_SCH) {
    WARN("no more place to schedule");
    return;
  }
  if (doitfirst) {
    while (schp > sys.sch) {
      *schp = *(schp - 1);
      schp--;
    }
  }
  schp->when   = time10ms() + dt;
  schp->opt    = opt;
  schp->conid  = conid;
  schp->funcid = funcid;
}
// --                -- initialization --
//                   --------------------

// -- nsmd_shmtouch -----------------------------------------------------
//    touch shared memory
// ----------------------------------------------------------------------
void
nsmd_shmtouch()
{
  time_t now = time(0);
  if (nsmd_sysp) nsmd_sysp->timevent = now;
  if (nsmd_memp) nsmd_memp->timevent = now;
}
// -- nsmd_shmget -------------------------------------------------------
//    get shared memory
//    ret = 0: successfully created
//    ret = ENOENT: successfully removed and created
//    ret = EEXIST: already existing a compatible one and can't be removed
//    ret = EACCES: already existing but no permission to write
//    ret = EPERM:  already existing but no permission to read
//    ret = EIDRM:  already existing but no permission to remove
//    ret = EINVAL: already existing but size is wrong
//      (shmget returns EINVAL only if size is smaller)
//    memp is set only when ret is 0 or EEXIST or EACCESS
// ----------------------------------------------------------------------
int
nsmd_shmget(int key, int siz, int prot, int& id, char** memp, time_t now)
{
  int ret = 0;

  // first try shmget to check if shm already exists
  LOG("getting shared memory (key=%d size=%d bytes)", key, siz);
  if ((id = shmget(key, siz, prot)) < 0) {
    switch (errno) {
      case ENOENT: break;  // normal
      case EINVAL: return EINVAL;
      case EACCES:
        id = shmget(key, siz, 0444); // try again with readonly
        if (id < 0) {
          if (errno == EACCES) {
            WARN("shmget permission denied key=%d", key);
            return EPERM;
          }
          ERRO("shmget/shmget(rdonly) key=%d", key); // unexpected
        }
        WARN("shmget readonly key=%d", key);
        ret = EACCES; // successfully shmget with readonly
        break;
      default:
        ERRO("shmget key=%d", key); // unexpected
    }
  }

  // if shm exist, check stat and remove if possible
  if (id >= 0) { // including the case with readonly
    shmid_ds ds;
    if (shmctl(id, IPC_STAT, &ds) < 0) {
      ERRO("shmget/shmctl(stat) key=%d", key); // unexpected
    }
    if (ds.shm_nattch == 0) {
      if (shmctl(id, IPC_RMID, 0) < 0) {
        // overriding ret = EPERM case: if nattch == 0, no point to read shm
        if (errno == EPERM) return EIDRM;
        ERRO("shmget/shmctl(rmid) key=%d", key); // unexpected
      } else {
        ret = ENOENT; // successfully removed
        id = -1;
      }
    } else if (ds.shm_segsz != siz) {
      WARN("shmget wrong size key=%d size %d != %d", key, ds.shm_segsz, siz);
      return EINVAL;
    } else {
      WARN("shmget key=%d existing with nattch=%d", key, ds.shm_nattch);
      ret = EEXIST;
    }
  }

  // create a new one if not exist or successfully removed
  if (id < 0) {
    if ((id = shmget(key, siz, prot | IPC_CREAT)) < 0) {
      ERRO("shmget/shmget key=%d", key); // unexpected
    }
  }

  *memp = (char*)shmat(id, 0, 0);
  if (*memp == (char*) - 1) ERRO("shmget/shmat key=%d"); // unexpected

  if (ret == EEXIST || ret == EACCES) { // not for touch or RMID
    id = -1;
  } else {
    ((NSMmem*)*memp)->ipaddr = nsmd_myip;
    ((NSMmem*)*memp)->pid = getpid();
    ((NSMmem*)*memp)->timstart = ((NSMmem*)*memp)->timevent = now;
  }
  return ret;
}
// -- nsmd_shmclean -----------------------------------------------------
//    set up initial values for the shared memory structure
// ----------------------------------------------------------------------
void
nsmd_shmclean()
{
  NSMsys& sys = *nsmd_sysp;
  NSMmem& mem = *nsmd_memp;
  int headlen = sizeof(int32_t) * 2 + sizeof(int64_t) * 2;

  memset(((char*)&sys) + headlen, 0, sizeof(NSMsys) - headlen);
  memset(((char*)&mem) + headlen, 0, sizeof(NSMmem) - headlen);
  for (NSMnod* nodp = sys.nod; nodp < sys.nod + NSMSYS_MAX_NOD; nodp++) {
    nodp->ipaddr = htonl(-1);
    nodp->noddat = htonl(-1);
  }
  for (NSMref* refp = sys.ref; refp < sys.ref + NSMSYS_MAX_REF; refp++) {
    refp->refnod = htons(-1);
    refp->refdat = htons(-1);
  }
  for (NSMdat* datp = sys.dat; datp < sys.dat + NSMSYS_MAX_DAT; datp++) {
    datp->dtpos = htonl(-1);
    datp->anext = htons(-1);
    datp->nnext = htons(-1);
  }
  for (NSMcon* conp = sys.con; conp < sys.con + NSMSYS_MAX_CON; conp++) {
    conp->sock = -1;
  }
  sys.afirst = htons(-1);
  sys.ready = -1;
}
// -- nsmd_setugid ------------------------------------------------------
//
// uid and gid to create MEM shared memory
//
// uid is chosen in the following priority from
//   1. "-u" option
//   2. NSMD2_USER environment variable
//   3. "nsm" user if exist
//   4. unchanged
//
// gid is chosen in the following priority from
//   1. "-g" option
//   2. NSMD2_GROUP environment variable
//   3. from user if it is given in the form "user.group"
//   4. from user's gid if user is given
//   5. "nsm" group if exist
//   6. unchanged
//
// ----------------------------------------------------------------------
void
nsmd_setugid()
{
  // -- get username and struct passwd
  errno = 0;
  char usrnam[256];
  const char* grpnamp = 0;
  usrnam[sizeof(usrnam) - 1] = 0;
  if (nsmd_usrnam) {
    strncpy(usrnam, nsmd_usrnam, sizeof(usrnam) - 1);
    if (grpnamp = strchr(usrnam, '.')) * (char*)grpnamp++ = 0;
  } else {
    strcpy(usrnam, "nsm");
  }
  struct passwd* pwdp = getpwnam(usrnam);
  if (! pwdp && nsmd_usrnam) {
    WARN("nsmd cannot find user %s\n", nsmd_usrnam);
    nsmd_destroy();
  }

  // -- get groupname and gid
  gid_t gid = -1;
  if (nsmd_grpnam) grpnamp = nsmd_grpnam;
  const char* grpnamp2 = grpnamp;

  if (! grpnamp && pwdp) {
    gid = pwdp->pw_gid;
  } else {
    if (! grpnamp2) grpnamp2 = "nsm";
    struct group* gidp = getgrnam(grpnamp2);
    if (! gidp) {
      if (grpnamp) {
        WARN("nsmd cannot find group %s\n", grpnamp);
        nsmd_destroy();
      }
    } else {
      gid = gidp->gr_gid;
    }
  }

  // -- set gid if needed
  if (gid >= 0) {
    if (setegid(gid) < 0) {
      if (grpnamp) {
        WARN("nsmd cannot set gid to group %s", grpnamp2);
        nsmd_destroy();
      } else {
        LOG("nsmd cannot set gid to group %s (ignored)", grpnamp2);
      }
    } else {
      LOG("nsmd set gid to group %s gid %d", grpnamp2, gid);
    }
  }

  // -- set uid if needed
  if (pwdp) { // simply ignore if "nsm" user does not exist
    if (seteuid(pwdp->pw_uid) < 0) {
      if (nsmd_usrnam) {
        WARN("nsmd cannot set uid to user %s\n", usrnam);
        nsmd_destroy();
      } else {
        LOG("nsmd cannot set uid to user %s\n", usrnam);
      }
    } else {
      LOG("nsmd set uid to user %s uid %d\n", usrnam, pwdp->pw_uid);
    }
  }
  return;
}
// -- nsmd_shmopen ------------------------------------------------------
//    open shared memory
// ----------------------------------------------------------------------
void
nsmd_shmopen(int shmkey, time_t now)
{
  // DBG("nsmd_shmopen(%d)", shmkey);

  int retsys = nsmd_shmget(shmkey,   sizeof(NSMsys), 0644, nsmd_shmsysid,
                           (char**)&nsmd_sysp, now);
  int euid = geteuid();
  int egid = getegid();
  nsmd_setugid();
  int retmem = nsmd_shmget(shmkey + 1, sizeof(NSMmem), 0664, nsmd_shmmemid,
                           (char**)&nsmd_memp, now);
  seteuid(euid);
  setegid(egid);

  if ((retsys == 0 || retsys == ENOENT) &&
      (retmem == 0 || retmem == ENOENT)) {
    if (retsys && retmem) {
      LOG("old shared memory removed");  // possible
    } else if (retsys) {
      WARN("old shared memory (SYS only) removed");  // unexpected but go
    } else if (retmem) {
      WARN("old shared memory (MEM only) removed");  // unexpected but go
    }
  } else if (retsys != retmem) {
    WARN("SYS and MEM memory are inconsistent ret=%d/%d\n", retsys, retmem);
    nsmd_destroy();
  } else if (retsys == EEXIST || retsys == EACCES) {
    NSMsys& sys = *nsmd_sysp;
    NSMmem& mem = *nsmd_memp;
    // pid/ipaddr/timstart/timevent are those unchanged by shmclean
    if (sys.pid == mem.pid && sys.ipaddr == mem.ipaddr &&
        sys.timstart == mem.timstart) {
      if (kill(sys.pid, 0) < 0 && errno == ESRCH) { // unexpected
        WARN("%s, %s: pid=%d, ip=%08x, t=%d/%d",
             "shared memory without process",
             "killing maybe still running clients",
             sys.pid, sys.ipaddr, sys.timstart, sys.timevent);
        nsmd_destroy_clients();
        sleep(1);
        sys.pid      = mem.pid      = getpid();
        sys.ipaddr   = mem.ipaddr   = nsmd_myip;
        sys.timstart = mem.timstart = now;
        sys.timevent = mem.timevent = now;
        WARN("setting up new sys/mem.pid=%d ip=%08x t=%d",
             sys.pid, sys.ipaddr, sys.timstart);

      } else {
        const char* msg = (retsys == EACCES) ? " by other account" : "";
        int exit_code = (retsys == EACCES) ? NSMD_ESHMPERM : NSMD_ESHMGET;
        time_t tstart = sys.timstart;
        tm tmstart;
        localtime_r(&tstart, &tmstart);
        int tdiff = (sys.timevent - sys.timstart);
        int tupdate = time(0) - sys.timevent;

        WARN("Other NSMD is running%s: pid=%d, " // no break
             "tstart=%04d.%02d.%02d-%02d:%02d:%02d up %d sec last %d sec ago",
             msg, sys.pid,
             tmstart.tm_year + 1900, tmstart.tm_mon + 1, tmstart.tm_mday,
             tmstart.tm_hour, tmstart.tm_min, tmstart.tm_sec,
             tdiff, tupdate);
        nsmd_destroy(exit_code);
      }
    } else {
      WARN("shared memory inconsistent: pid=%d/%d ip=%08x/%08x t=%d/%d",
           sys.pid, mem.pid, sys.ipaddr, mem.ipaddr,
           sys.timstart, sys.timevent);
      nsmd_destroy();
    }
  } else {
    WARN("shmget unexpected return code %d/%d", retsys, retmem);
    nsmd_destroy();
  }

  nsmd_shmclean();

  LOG("nsmd shared memory cleaned up");
}
// -- nsmd_udpopen ------------------------------------------------------
// 1. open UDP socket and bind
// 2. loop over the interface to look for the adrdress
// ----------------------------------------------------------------------
int
nsmd_udpopen(int port, SOCKAD_IN& bcast_sockad, int& netmask)
{
  int sock;
  int TRUE = 1;
  int size = NSMD_UDPBUFSIZ;
  char adrbuf[32], brdbuf[32];
  char hostname[256];
  struct hostent* hp;
  SOCKAD_IN udpaddr;

  // open socket
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    ERRO("udpopen: socket");

  // set socket size, etc
  if (SOCKOPT(sock, SO_REUSEADDR, TRUE) < 0)
    ERRO("udpopen: sockopt reuse"); // unexpected
  // set socket size, etc
  if (SOCKOPT(sock, SO_SNDBUF, size) < 0)
    ERRO("udpopen: sockopt sndbuf"); // unexpected
  if (SOCKOPT(sock, SO_RCVBUF, size) < 0)
    ERRO("udpopen: sockopt rcvbuf"); // unexpected

  // bind
  memset(&udpaddr, 0, sizeof(udpaddr));
  //ADDR_IP(udpaddr) = INADDR_ANY;
  udpaddr.sin_addr.s_addr = INADDR_ANY;

  if (BIND(sock, &udpaddr, port) < 0) {
    close(sock);
    ERRO("udpopen: bind"); // unexpected
  }

  // broadcast option
  if (SOCKOPT(sock, SO_BROADCAST, TRUE) < 0)
    ERRO("udpopen: sockopt broadcast"); // unexpected

  // broadcast address (only for the first one excluding localhost)
  struct ifconf ifc;
  struct ifreq* ifr;
  int ifrgap;
  const int bufsiz = 4096;
  char* p, buf[4096];
  memset(buf, 0, bufsiz);
  ifc.ifc_len = bufsiz;
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, (char*)&ifc) < 0)
    ERRO("udpoepn: ioctl getconf"); // unexpected

  for (struct ifreq* ifr = ifc.ifc_req;
       (char*)ifr < (char*)ifc.ifc_req + ifc.ifc_len;
       ifr = (struct ifreq*)((char*)ifr + ifrgap)) {

#if defined(__FreeBSD__) || defined(MVME5100)
    // This ugly code is taken for NSM v1 from bind 4.8.x for FreeBSD
    // VxWorks on MVME5100 requires this, too.
    // NSM v2 is not tested on FreeBSD/MVME5100
#define xxx_max(a,b) ((a)>(b)?(a):(b))
#define xxx_size(p)  xxx_max((p).sa_len,sizeof(p))
    ifrgap = sizeof(ifr->ifrname) + xxx_size(ifr->ifr_addr);
#else
    ifrgap = sizeof(*ifr);
#endif

    if (! ifr->ifr_name[0]) break;

    if (ifr->ifr_addr.sa_family != AF_INET) continue;

    if (ioctl(sock, SIOCGIFFLAGS, (char*)ifr) < 0)
      ERRO("udpopen: ioctl getflags for interface <%s>", ifr->ifr_name);

    if ((ifr->ifr_flags & IFF_UP) == 0) {
      LOG("interface <%s> is down", ifr->ifr_name);
      continue;
    }
    if ((ifr->ifr_flags & IFF_BROADCAST) == 0) {
      LOG("interface <%s> does not support broadcast", ifr->ifr_name);
      continue;
    }

    SOCKAD_IN if_sockad, mask_sockad;
    if (ioctl(sock, SIOCGIFADDR, (char*)ifr) < 0) {
      LOG("interface <%s> has no address", ifr->ifr_name);
      continue;
    }
    memcpy(&if_sockad, (SOCKAD_IN*)&ifr->ifr_addr, sizeof(SOCKAD_IN));

    if (ADDR_IP(if_sockad) != nsmd_myip) {
      LOG("interface <%s> addr=%s and %s does not match",
          ifr->ifr_name, ADDR_STR(if_sockad), ADDR_STR(nsmd_sockad));
      continue;
    }

    if (ioctl(sock, SIOCGIFNETMASK, (char*)ifr) < 0) {
      ERRO("interface <%s> has no netmask", ifr->ifr_name);
    }
    memcpy(&mask_sockad, (SOCKAD_IN*)&ifr->ifr_addr, sizeof(SOCKAD_IN));
    netmask = ADDR_IP(mask_sockad);

    if (ioctl(sock, SIOCGIFBRDADDR, (char*)ifr) < 0) {
      ERRO("udpopen: ioctl getbrdaddr <%s>", ifr->ifr_name);
    }
    memcpy(&bcast_sockad, (SOCKAD_IN*)&ifr->ifr_broadaddr, sizeof(SOCKAD_IN));
    bcast_sockad.sin_family = AF_INET;
    bcast_sockad.sin_port = htons((short)port);

    LOG("interface <%s> addr=%s port=%d sock=%d",
        ifr->ifr_name, ADDR_STR(if_sockad), port, sock);
    LOG("interface <%s> bcast=%s netmask=%s",
        ifr->ifr_name, ADDR_STR(bcast_sockad), ADDR_STR(mask_sockad));
    return (sock);
  }
  nsmd_destroy(NSMD_ENONETIF);
  return (NSMD_ENONETIF); // unreachable
}
// -- nsmd_tcpopen ------------------------------------------------------
// ----------------------------------------------------------------------
int
nsmd_tcpopen(int port)
{
  int TRUE = 1;
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    ERRO("tcpopen: socket");        // unexpected
  if (SOCKOPT(sock, SO_REUSEADDR, TRUE) < 0)
    ERRO("tcpopen: sockopt reuse"); // unexpected
  if (BIND(sock, &nsmd_sockad, port) < 0) {
    if (errno == EADDRINUSE) {
      LOG("address already in use, probably another NSMD is already running");
      nsmd_destroy(NSMD_EINUSE);
    } else {
      ERRO("tcpopen: bind"); // unexpected
    }
  }
  if (listen(sock, NSMSYS_MAX_CON) < 0)
    ERRO("tcpopen: listen"); // unexpected

  LOG("LISTEN: addr=%s port=%d sock=%d", ADDR_STR(nsmd_sockad), port, sock);
  return sock;
}
// -- nsmd_init ---------------------------------------------------------
//    initialization main
//    (void return -- any error will terminate the program)
// ----------------------------------------------------------------------
void
nsmd_init(int dryrun)
{
  // trap signals
  signal(SIGINT,  nsmd_destroy);
  signal(SIGTERM, nsmd_destroy);
  signal(SIGCHLD, nsmd_destroy);
  signal(SIGPIPE, nsmd_brokenpipe);
  signal(SIGHUP,  nsmd_dumpall);
  signal(SIGUSR1, nsmd_dumpall);

  // UDP and TCP socket (any error would abort the program)
  SOCKAD_IN bcast_sockad;
  int netmask;
  int udpsock = nsmd_udpopen(nsmd_port, bcast_sockad, netmask);
  int tcpsock = nsmd_tcpopen(nsmd_port);

  // open shared memory (any error would abort the program)
  time_t now = time(0);
  nsmd_shmopen(nsmd_shmkey, now);

  if (dryrun) {
    close(udpsock);
    close(tcpsock);
    if (nsmd_sysp) shmdt(nsmd_sysp);
    if (nsmd_memp) shmdt(nsmd_memp);
    nsmd_sysp = 0;
    nsmd_memp = 0;
    return;
  }

  // set up shared memory
  NSMsys& sys = *nsmd_sysp;
  NSMcon& udpcon = sys.con[NSMCON_UDP];
  NSMcon& tcpcon = sys.con[NSMCON_TCP];
  memset(&udpcon, 0, sizeof(udpcon));
  memset(&tcpcon, 0, sizeof(tcpcon));
  udpcon.sock = udpsock;
  tcpcon.sock = tcpsock;
  udpcon.sockad = bcast_sockad;
  tcpcon.sockad = nsmd_sockad;
  udpcon.pid  = tcpcon.pid = -1;
  udpcon.nid  = tcpcon.nid = -1;
  udpcon.timstart = 0;   // NSM2 timstart is unknown yet
  tcpcon.timstart = now;
  udpcon.pid = tcpcon.pid = sys.pid;
  sys.ncon = 2;
  sys.master = sys.deputy = NSMCON_NON;
  sys.version  = NSM_DAEMON_VERSION;
  sys.protocol = NSM_PROTOCOL_VERSION;
  sys.priority = nsmd_priority;

  //sys.master = NSMCON_TCP; //
  nsmd_init_count = NSMD_INITCOUNT_FIRST;
  nsmd_schedule(NSMCON_NON, NSMSCH_INITBCAST, 0, 0, 0);

  // OK
  return;
}
//                   ---------------
// --                -- send calls --
//                   ----------------
// -- nsmd_udpsend ------------------------------------------------------
//
// sockadp=0 for broadcast
// npar!=0 to use bufp->dat to send int32_t*npar
// len!=0 to use bufp->dat as raw stream
static void
nsmd_udpsend(SOCKAD_IN* sockadp, int req, int par1, int par2,
             NSMudpbuf* bufp = 0, int len = 0, int npar = 0, uint32_t* pars = 0)
{
  NSMsys& sys = *nsmd_sysp;
  NSMcon& udp = sys.con[NSMCON_UDP];
  if (len > NSM_UDPDATSIZ) ASRT("udpsend too long %d", len);
  static int udpseq = 0;
  NSMudpbuf buf;
  if (! bufp) bufp = &buf;
  bufp->req  = req - NSMCMD_UDPOFFSET;
  bufp->seq  = udpseq++;
  bufp->npar = npar;
  bufp->par1 = ntohs(par1);
  bufp->par2 = ntohs(par2);

  /*
  if (len > 4) {
    DBG("udpsend: len=%d %02x %02x %02x %02x", len,
  (uint8_t)bufp->dat[0], (uint8_t)bufp->dat[1],
  (uint8_t)bufp->dat[2], (uint8_t)bufp->dat[3]);
  }
  */

  // make sockad
  if (! sockadp) sockadp = &udp.sockad;

  len += (bufp->dat - (char*)bufp) + npar * sizeof(int32_t);
  for (int i = 0; i < npar; i++) {
    *(int32_t*)&buf.dat[i * sizeof(int32_t)] = ntohl(pars[i]);
  }

  int ret = sendto(udp.sock, (char*)bufp, len, 0,
                   (SOCKAD*)sockadp, sizeof(SOCKAD));
  if (ret < 0) ERRO("udpsend");
  udp.ocnt++;
  udp.osiz += ret;
}

#if 0
static void
nsmd_udpsend_not(SOCKAD_IN* sockadp, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  NSMcon& udp = sys.con[NSMCON_UDP];
  NSMudpbuf buf;
  static int udpseq;

  // check length
  int len = dmsg.npar * sizeof(int32_t) + dmsg.len;
  if (len > NSM_UDPDATSIZ) {
    ASRT("udpsend too long npar=%d len=%d", dmsg.npar, dmsg.len);
  }
  len += (&buf.dat[0] - (char*)&buf);

  // make sockad
  if (! sockadp) {
    sockadp = &udp.sockad;
  }

  // build send buffer
  buf.req = dmsg.req - NSMCMD_UDPOFFSET;
  buf.seq = ntohs(udpseq++);
  buf.npar = dmsg.npar;
  for (int i = 0; i < dmsg.npar; i++) {
    *(int32_t*)&buf.dat[i * sizeof(int32_t)] = ntohl(dmsg.pars[i]);
  }
  if (dmsg.len) {
    memcpy(&buf.dat[dmsg.npar * sizeof(int32_t)], dmsg.datap, dmsg.len);
  }

  int ret = sendto(udp.sock, (char*)&buf, len, 0,
                   (SOCKAD*)sockadp, sizeof(*sockadp));
  if (ret < 0) ERRO("udpsend");
  udp.ocnt++;
  udp.osiz += ret;
}
#endif
// -- nsmd_freeq --------------------------------------------------------
static void
nsmd_freeq(NSMDtcpq* q)
{
  // otherwise queue entry can be freed
  //DBG("freeq before f=%08x(%08x) l=%08x q=%08x",
  // nsmd_tcpqfirst, nsmd_tcpqfirst?nsmd_tcpqfirst->prevp:0, nsmd_tcpqlast, q);
  if (q->prevp) {  // not the first one
    q->prevp->nextp = q->nextp;
  } else {         // the first one
    nsmd_tcpqfirst = q->nextp;
  }
  if (q->nextp) {  // not the last one
    q->nextp->prevp = q->prevp;
  } else {         // the last one
    nsmd_tcpqlast = q->prevp;
  }
  //DBG("freeq after f=%08x(%08x) l=%08x q=%08x",
  // nsmd_tcpqfirst, nsmd_tcpqfirst?nsmd_tcpqfirst->prevp:0, nsmd_tcpqlast, q);

  if (q->datap) {
    nsmd_free("tcpfreeq(longbuf)", q->datap - q->npar * sizeof(int32_t));
  }

  nsmd_free("tcpfreeq(q)", (char*)q);
}
// -- nsmd_tcpwriteq ----------------------------------------------------
// process queue in two steps
// 1. convert to buffer
// 2. send it
// ----------------------------------------------------------------------
static int
nsmd_tcpwriteq()
{
  NSMsys& sys = *nsmd_sysp;
  int ret; // for select and write

  //DBG("tcpwriteq: entry");

  if (! nsmd_tcpqfirst && ! nsmd_tcpqlast) return 0; // nothing in the queue
  if (! nsmd_tcpqfirst) ASRT("tcpwriteq: qfirst is broken");
  if (! nsmd_tcpqlast)  ASRT("tcpwriteq: qlast is broken");

  // find the next one in the queue to send
  NSMDtcpq* q   = nsmd_tcpqfirst;
  NSMDtcpq* qws = 0;
  while (q) {
    if (! ExistConid(q->conid)) {
      ASRT("no conid=%d dmsg.req=%x", q->conid, q->req);
    }
    if (sys.con[q->conid].status == NSMDC_OK) break;
    if (sys.con[q->conid].status == NSMDC_JC) break;
    if (sys.con[q->conid].status == NSMDC_WS && ! qws) qws = q;
    q = q->nextp;
  }
  if (! q) q = qws;
  if (! q) {
    DBG("tcpwriteq: no queue");
    return 0;
  }
  NSMcon& con = sys.con[q->conid];

  // check if I can write
  timeval tv;
  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(con.sock, &fdset);
  memset(&tv, 0, sizeof(tv));

  while (1) { // just for EINTR and EAGAIN
    if ((ret = select(con.sock + 1, 0, &fdset, 0, &tv)) >= 0) break;
    if (errno == EINTR || errno == EAGAIN) continue;
    ERRO("tcpwriteq: select"); // unexpected
  }

  // if timeout, mark it as WAIT-SELECT
  if (ret == 0) { // timeout
    con.osel++;
    if (con.status == NSMDC_WS) return 0; // don't try again
    con.status = NSMDC_WS;
    return 1; // would like to try again
  } else {
    con.status = NSMDC_OK;
  }

  // copy to buffer if total length is short (happens if datap is in sys)
  int headparlen = sizeof(NSMtcphead) + q->npar * sizeof(int32_t);
  if (q->datap && q->msglen < NSM_TCPTHRESHOLD) {
    if (q->pos) ASRT("tcpwriteq: q->pos(%d) should be 0", q->pos);
    memcpy(q->buf + headparlen, q->datap, q->msglen - headparlen);
    q->datap = 0;
  }

  // set up write size and ptr
  enum { WSHORT, WHEAD, WDATA } wtyp;
  const char* writep;
  int writelen;
  if (! q->datap) {
    wtyp     = WSHORT;
    writep   = q->buf + q->pos;
    writelen = q->msglen - q->pos;
  } else if (q->pos < headparlen) {
    wtyp     = WHEAD;
    writep   = q->buf + q->pos;
    writelen = headparlen - q->pos;
  } else {
    wtyp     = WDATA;
    writep   = q->datap + (q->pos - headparlen);
    writelen = q->msglen - q->pos;
  }

  // and actual write
  while (1) { // just for EINTR and EAGAIN
    if ((ret = write(con.sock, writep, writelen)) >= 0) break;
    if (errno == EINTR || errno == EAGAIN) continue;
    // 20170927 this is not unexpected
    WARN("tcpwriteq(%d,%d) write %d bytes failed: %s",
         con.sock, q->conid, writelen, strerror(errno));
    nsmd_destroyconn(con, 1, "tcpwriteq");
    return 1;
  }

  {
    NSMtcphead& head = *(NSMtcphead*)q->buf;
    int* p = (int*)(q->buf + sizeof(NSMtcphead));
    int npar = head.npar;
    int req  = ntohs(head.req);
    int len  = ntohs(head.len);
    char parbuf[256];
    switch (npar) {
      case 0: strcpy(parbuf, "npar=0"); break;
      case 1: sprintf(parbuf, "p=[%d]", ntohl(p[0])); break;
      case 2: sprintf(parbuf, "p=[%d,%d]",
                        ntohl(p[0]), ntohl(p[1])); break;
      default: sprintf(parbuf, "n=%d p=[%d,%d]",
                         npar, ntohl(p[0]), ntohl(p[1])); break;
    }
    DBG("tcpwriteq(%d,%d) req=%x len=%d %s buf=%x wp=%x(%d/%d)",
        con.sock, q->conid,
        req, len, parbuf, q->buf, writep, ret, writelen);

  }

  if (ret == 0) {
    WARN("tcpwriteq: write returns 0 for con=%d", q->conid);
    nsmd_destroyconn(con, 1, "tcpwriteq");
    return 1;
  }

  // for a large datap, keep the queue to be called again
  if (ret == writelen && wtyp == WHEAD) {
    q->pos = headparlen;
    return 1; // call me again
  }

  // if not completed, keep the queue to be called again
  if (ret < writelen) {
    q->pos += writelen;
    return 1; // call me again
  }

  // both allocated data and queue itself are freed
  nsmd_freeq(q);

  return 1; // call me again to check if there's another queue entry
}
// -- nsmd_tcpsend ------------------------------------------------------
//
// no return code, all error handling should be done inside tcpsend,
// since there are too many tcpsend calls.
//
// tcpsend will add the message at the end of the queue, unless it
// is called with express=1
// ----------------------------------------------------------------------
static void
nsmd_tcpsend(NSMcon& con, NSMdmsg& dmsg, NSMDtcpq* qptr, int beforeafter)
{
  NSMsys& sys = *nsmd_sysp;
  int req = dmsg.req;
  int len = dmsg.len;

  char reqstr[256];
  //DBG("tcpsend req=%04x", dmsg.req);

  // name of the request
  if (req >= NSMCMD_FIRST && req <= NSMCMD_LAST) {
    NSMcmdtbl_t& cmd = nsmd_cmdtbl[req - NSMCMD_FIRST];
    strcpy(reqstr, cmd.name);
  } else {
    sprintf(reqstr, "[%04x]", req);
  }

  // check conn
  int conid = CON_ID(con);
  if (conid < 2 || conid >= sys.ncon) {
    WARN("tcpsend: no conn(%d)? req=%s len=%d", conid, reqstr, len);
    // not sure if it is expected.  In nsmd1 the comment says:
    // --- it is possible that the connection is lost but it's not updated
    //     in the schedule list...  ---
    return;
  }

  // print the request
  int dispflag = 1;
  if ((req == NSMCMD_SYSCPYMEM || req == NSMCMD_REQCPYMEM) &&
      (!DBGBIT(4) || (sys.ready > 0) || len == NSMD_TCPSENDSIZ)) dispflag = 0;
  if ((req == NSMCMD_USRCPYMEM || req == NSMCMD_FLUSHMEM ||
       req == NSMCMD_PING || req == NSMCMD_PONG) && !DBGBIT(6)) dispflag = 0;
  if (DBGBIT(1) || (reqstr[0] == '[' && ! DBGBIT(10))) dispflag = 0;
  if (dispflag) {
    LOG("send %s(%d)=>%s", reqstr, dmsg.seq, ADDR_STR(con.sockad));
  }

  // skip sending to removed or temporarily unavailable destination
  switch (con.status) {
    case NSMDC_NA:
    case NSMDC_WA: LOG("tcpsend: skipping since not available"); // no break;
    case NSMDC_RM: return;
  }

  // reconnect if it is available again
#if READYtoIMPLEMENT
  // maybe it shouldn't return if reconnect is OK
  if (conn.sock < 0) {
    if (con.status == NSMDC_OK) {
      nsmd_reconnect(con);
      return; // ???
    } else {
      WARN("invalid socket, conid=%d", CON_ID(con));
      con.status = NSMDC_NA;
      return;
    }
  }
#endif

  //DBG("con.pid = %d", con.pid);

  // send signal if local
  if (con.pid > 0) {
    int ret;
    while (1) {
      if (con.sigobs) {
        ret = kill((pid_t)con.pid, SIGUSR1);
      } else {
#ifdef SIGRTMIN
        const union sigval sv = { 0 };
        ret = sigqueue((pid_t)con.pid, SIGRTMIN, sv);
        //ret = sigqueue((pid_t)con.pid, SIGRTMIN, (const union sigval)0);
#else
        ERRO("SIGRTMIN undefined");
#endif
      }
      if (ret >= 0) break;
      if (errno == EINTR || errno == EAGAIN) continue;
      if (errno == ESRCH) { // remove client if process is gone
        LOG("DESTROY non-existing process pid=%d", con.pid);
        nsmd_destroyconn(con, 1, "tcpsend");
        return;
      }
      ERRO("cannot send signal to pid=%d", con.pid);
    }
  }

  // basic checks
  if (nsmd_tcpqfirst && ! nsmd_tcpqlast || nsmd_tcpqlast && ! nsmd_tcpqfirst) {
    ASRT("tcpsend: only tcpq%s is null", nsmd_tcpqfirst ? "last" : "first");
  } else if (qptr && ! nsmd_tcpqfirst) {
    ASRT("tcpsend: missing qptr");
  }

  // allocate a new queue entry and put it at somewhere in the queue
  NSMDtcpq* q;
  if (qptr && beforeafter == 0) {
    q = qptr;
  } else {
    q = (NSMDtcpq*)nsmd_malloc("tcpsend", sizeof(*q));
  }

  // the default is to add at the end
  if (! qptr) {
    qptr = nsmd_tcpqlast;
    beforeafter = 1;
  }

  // reconnect the queue entries
  if (! nsmd_tcpqfirst) { // no queue yet, make a new one
    nsmd_tcpqfirst = nsmd_tcpqlast = q;
    q->prevp = 0;
    q->nextp = 0;
  } else if (beforeafter < 0) { // before qptr
    q->prevp = qptr->prevp;
    q->nextp = qptr;
    if (q->prevp) q->prevp->nextp = q;
    if (qptr == nsmd_tcpqfirst) nsmd_tcpqfirst = q;
    qptr->prevp = q;
  } else if (beforeafter > 0) { // after qptr
    q->nextp = qptr->nextp;
    q->prevp = qptr;
    if (q->nextp) q->nextp->prevp = q;
    if (qptr == nsmd_tcpqlast) nsmd_tcpqlast = q;
    qptr->nextp = q;
  }
  /* 20170927 commented out
  DBG("tcpsend(%d,%d) f=%08x(%08x) l=%08x q=%08x %d",
      con.sock, CON_ID(con),
      nsmd_tcpqfirst, nsmd_tcpqfirst?nsmd_tcpqfirst->prevp:0, nsmd_tcpqlast,
      q, beforeafter);
  */

  // setup the queue entries
  q->conid = CON_ID(con);
  q->req   = dmsg.req;
  q->npar  = dmsg.npar;
  q->msglen = sizeof(NSMtcphead) + dmsg.npar * sizeof(int32_t) + dmsg.len;
  q->pos   = 0;

  // setup the header entries
  NSMtcphead& head = *(NSMtcphead*)q->buf;
  head.src  = htons(dmsg.src);
  head.dest = htons(dmsg.dest);
  head.req  = htons(dmsg.req);
  head.seq  = htons(dmsg.seq);
  head.opt  = dmsg.opt;
  head.npar = dmsg.npar;
  head.len  = htons(dmsg.len);

  // setup parameters and choose data buffer or pointer according to the size
  // (or if sys data, use always pointer)

  char* datap = (char*)&head + sizeof(head);
  for (int i = 0; i < dmsg.npar; i++) {
    *(int32_t*)datap = htonl(dmsg.pars[i]);
    datap += sizeof(int32_t);
  }

  if (q->msglen <= NSM_TCPTHRESHOLD &&
      !((char*)&sys <= dmsg.datap && dmsg.datap < (char*)(&sys + 1))) {
    q->datap = 0;
    memcpy(datap, dmsg.datap, dmsg.len);
  } else {
    q->datap = dmsg.datap;
    dmsg.datap = 0; // clear it to avoid freeing
  }
}
//                   ----------------
// --                -- conn calls --
//                   ----------------
// -- nsmd_tcpconnect ---------------------------------------------------
// ----------------------------------------------------------------------
int
nsmd_tcpconnect(SOCKAD_IN& sockad, const char* contype)
{
  LOG("CONNECT to %s (type=%s port=%d)",
      ADDR_STR(sockad), contype, ntohs(sockad.sin_port));

  // open a socket and set options
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) ERRO("tcpconnect: socket");
  int TRUE = 1;
  int SIZE = nsmd_tcpsocksiz;
  if (SOCKOPT(sock, SO_REUSEADDR, TRUE) < 0)
    ERRO("tcpconnect: sockopt reuseaddr"); // unexpected
  if (SOCKOPT(sock, SO_RCVBUF, SIZE) < 0)
    ERRO("tcpconnect: sockopt rcvbuf"); // unexpected
  if (SOCKOPT(sock, SO_SNDBUF, SIZE) < 0)
    ERRO("tcpconnect: sockopt sndbuf"); // unexpected

  uint sizbuf = SIZE;
  uint sizlen = sizeof(SIZE);
  if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sizbuf, &sizlen) < 0)
    ERRO("tcpconnect: getsockopt sndbuf"); // unexpected
  if (sizbuf == 2 * SIZE) {
    LOG("TCP send buffer size %d\n", SIZE);
  } else {
    LOG("TCP send buffer size %d (truncated from %d)\n", sizbuf / 2, SIZE);
  }

  // allow timeout for connect
  int flagsav = fcntl(sock, F_GETFL);
  fcntl(sock, F_SETFL, O_NONBLOCK | flagsav);

  // connect
  int errno_prev = 0;
  while (connect(sock, (SOCKAD*)&sockad, sizeof(sockad)) < 0) {
    if (errno != errno_prev) LOG("CONNECT %s", strerror(errno));
    errno_prev = errno;

    switch (errno) {
#if EAGAIN != EWOULDBLOCK
      case EWOULDBLOCK:
#endif
      case EAGAIN:
      case EALREADY:
      case EINPROGRESS:
        DBG("tcpconnect in progress");
        sleep10ms(10); // 100ms
        continue;

      case EISCONN: // already connected (for solaris)
        DBG("CONNECT EISCONN");
        break;

      case EPIPE:
      case ECONNREFUSED:
      case EHOSTUNREACH:
      case ENETUNREACH:
      case ETIMEDOUT:
        LOG("CONNECT error %s", strerror(errno));
        close(sock);
        return -1;

      default:
        ERRO("tcpconnect connect");
    }
    break;
  }
  DBG("CONNECT done");

  // wait until connect is complete
  struct timeval tv;
  fd_set fdset;
  tv.tv_sec  = NSMD_CONNECT_TIMEOUT;
  tv.tv_usec = 0;
  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  switch (select(sock + 1, 0, &fdset, 0, &tv)) {
    case -1:
      ERRO("tcpconnect select");
    case 0:
      LOG("TIMEOUT connection to %s", ADDR_STR(sockad));
      close(sock);
      return -1;
  }
  fcntl(sock, F_SETFL, flagsav);
  DBG("CONNECT sock=%d", sock);
  return sock;
}
// -- nsmd_newconn ------------------------------------------------------
// - TCP connection is made only to master/deputy and
//   called from do_ackdaemon and do_newmaster
// - conid = sys.ncon in do_newmaster to create a new connection,
//   and sys.ncon++ for a successful connection, right after this call
//   in nsmd_do_ackdaemon or nsmd_do_newmaster
//
// ----------------------------------------------------------------------
int
nsmd_newconn(int conid, int32_t ip, const char* contype)
{
  NSMsys& sys = *nsmd_sysp;
  NSMcon& con = sys.con[conid];

  // connect
  SOCKAD_IN sockad = sys.con[NSMCON_TCP].sockad; // just as a template
  memset(&sockad.sin_addr, 0, sizeof(sockad.sin_addr));
  sockad.sin_addr.s_addr = ip;
  int sock = nsmd_tcpconnect(sockad, contype);
  if (sock < 0) return -1; // message is alrady printed

  // setup con
  memset(&con, 0, sizeof(con));
  con.sock   = sock;
  con.sockad = sockad;
  con.pid    = -1;
  con.nid    = -1;
  con.status = NSMDC_JC; // just connected, not ready yet
  con.ready  = sys.ready <= 0 ? 0 : sys.ready;
  con.timevent = time(0);
  sys.sock_updated  = 1;

  // clear recvbuf
  int coni  = CON_ID(con) - NSMCON_OUT; // instead of conid
  recv_bsizbuf[coni] = 0;

  return sock;
}
// -- nsmd_delconn ------------------------------------------------------
//
// ----------------------------------------------------------------------
void
nsmd_delconn(NSMcon& con)
{
  NSMsys& sys = *nsmd_sysp;
  const int conid = CON_ID(con);

  shutdown(con.sock, 2);
  close(con.sock);

  // sch list and push list have to be updated
  //   :
  //   :
  //   :

  // remove or adjust master/deputy
  if (sys.master == conid) { LOG("removing master"); sys.master = NSMCON_NON; }
  if (sys.master >  conid) sys.master--;
  if (sys.deputy == conid) { LOG("removing deputy"); sys.deputy = NSMCON_NON; }
  if (sys.deputy >  conid) sys.deputy--;

  // shift the connection list
  sys.ncon--;
  // bug! conid should not be modified as it is used to manipulate
  // tcpq -- for (; conid < sys.ncon; conid++) sys.con[conid] = sys.con[conid + 1];
  for (int i = conid; i < sys.ncon; i++) sys.con[i] = sys.con[i + 1];
  memset(&sys.con[sys.ncon], 0, sizeof(NSMcon));
  memset(&sys.con[sys.ncon].sockad.sin_addr, -1, sizeof(SOCKAD_IN));
  sys.con[sys.ncon].sock = -1;

  // clear and shift tcpq
  NSMDtcpq* q   = nsmd_tcpqfirst;
  while (q) {
    if (q->conid == conid) {
      NSMDtcpq* p = q;
      q = q->nextp;
      nsmd_freeq(p);
    } else {
      if (q->conid >  conid) q->conid--;
      q = q->nextp;
    }
  }

  // clear and shift recvbuf
  int coni = conid - NSMCON_OUT; // instead of conid
  if (recv_shortbufp[coni]) nsmd_free("delconn", recv_shortbufp[coni]);
  if (recv_longbufp[coni])  nsmd_free("delconn", recv_longbufp[coni]);
  for (int i = conid; i < sys.ncon; i++) {
    coni = i - NSMCON_OUT;
    recv_shortbufp[coni] = recv_shortbufp[coni + 1];
    recv_longbufp[coni]  = recv_longbufp[coni + 1];
    recv_bsizbuf[coni]   = recv_bsizbuf[coni + 1];
  }
  coni = sys.ncon - NSMCON_OUT;
  recv_shortbufp[coni] = recv_shortbufp[coni + 1];
  recv_longbufp[coni]  = recv_longbufp[coni + 1];
  recv_bsizbuf[coni] = 0;

  // and make it dirty to update select fdset
  sys.sock_updated = 1;
}
// -- nsmd_destroyconn --------------------------------------------------
//
// - if con is local => delclient
// - otherwise con is MASTER/DEPUTY or I am MASTER/DEPUTY
// - con is MEMBER and I'm a MEMBER => unexpected, delete con
// - con is MEMBER and I'm a
// ----------------------------------------------------------------------
static void
nsmd_destroyconn(NSMcon& con, int delcli, const char* bywhom)
{
  NSMsys& sys = *nsmd_sysp;
  int itslocal = ConIsLocal(con);
  int sock_save = con.sock;

  LOG("destroyconn by %s sock=%d at %s",
      bywhom, sock_save,
      itslocal ? "local" : ADDR_STR(con.sockad));
  DBG("con.nid = %d\n", con.nid);

  // never send anything to this connection from now on
  con.status = NSMDC_RM;

  if (itslocal) {
    if (con.nid == NSMSYS_MAX_NOD) {
      ;
    } else if (con.nid >= 0) {
      NSMdmsg dmsg;
      memset(&dmsg, 0, sizeof(dmsg));
      dmsg.req = NSMCMD_DELCLIENT;
      dmsg.src = con.nid;
      dmsg.dest = -1;
      nsmd_do_delclient(con, dmsg);
    } else if (sys.ready <= 0) {
      LOG("nsmd is not ready yet");
    } else {
      WARN("node is not found for local connection");
    }
  } else { // it's not local
    int conid = CON_ID(con);

    // delete client nodes that belong to con
    if (delcli) {
      NSMdmsg dmsg;
      memset(&dmsg, 0, sizeof(dmsg));
      dmsg.req = NSMCMD_DELCLIENT;
      dmsg.src = -1;
      nsmd_do_delclient(con, dmsg);
    }

    if (NoMaster()) {
      if (ConIsDeputy(con)) {
        sys.deputy = NSMCON_NON;
      }
      LOG("destroyconn: master is missing");
    } else if (ConIsMaster(con) && NoDeputy()) {
      nsmd_init_count = NSMD_INITCOUNT_SECOND;
      sys.master = NSMCON_NON;
    } else if (ConIsMaster(con)) {
      sys.master = sys.deputy;
      sys.deputy = NSMCON_NON;
      LOG("new master=%s", IamMaster() ? "me" :
          ADDR_STR(sys.con[sys.master].sockad));
    } else if (ConIsDeputy(con)) {
      sys.deputy = NSMCON_NON;
    } else if (!IamMaster() && !IamDeputy()) {
      WARN("destroyconn: disappeared con=%d is not a MASTER/DEPUTY",
           &con - sys.con);
    }
  }

  // if not equal, it might be already removedby do_delclient
  if (sock_save != con.sock) {
    ASRT("con.sock has changed from %d to %d", sock_save, con.sock);
  } else {
    nsmd_delconn(con);
  }

  // connection to local client connecting before ready
  if (itslocal || sys.ready <= 0) {
    return;
  }

  // find new deputy if I am a MASTER
  // or leave the deputy undefined until receiving NEWMASTER
  if (! itslocal && IamMaster() && NoDeputy()) { // if I am a new MASTER
    int prio = -1;
    uint32_t ip_h = (uint32_t) - 1;
    for (int conid = NSMCON_OUT; conid < sys.ncon; conid++) {
      if (ConidIsLocal(conid)) continue;
      int    newprio = sys.con[conid].priority;
      uint32_t newip_h = ntohl(ADDR_IP(sys.con[conid].sockad));
      if (newprio > prio || (newprio == prio && newip_h < ip_h)) {
        sys.deputy = conid;
        ip_h = newip_h;
        prio = newprio;
      }
    }
    if (! NoDeputy()) {
      // just send NEWMASTER to new DEPUTY
      // and rest of the reconnections will happen in do_ready
      LOG("new deputy=%s", ADDR_STR(sys.con[sys.deputy].sockad));
      nsmd_setup_daemon(sys.con[sys.deputy]);
    } else {
      LOG("no new deputy");
    }
  }

  // find new master if both MASTER/DEPUTY are gone
  // declare!
  if (NoMaster()) {
    sys.ready = -1;
    nsmd_init_count = NSMD_INITCOUNT_SECOND;
    nsmd_schedule(NSMCON_NON, NSMSCH_INITBCAST, 0, 0, 1);
  }
}
// --                 -- sch calls --
//                    ---------------
// -- nsmd_sch_initbcast ------------------------------------------------
int
nsmd_sch_initbcast(int16_t, int32_t)
{
  NSMsys& sys = *nsmd_sysp;
  int next = -1;

  // check the existence of master and deputy
  if (! NoMaster())
    DBG("initbcast: sys.master=%d", sys.master);
  if (sys.master >= NSMCON_TCP) {
    LOG("initialized as %s",
        IamMaster() ? "MASTER" : (IamDeputy() ? "DEPUTY" : "MEMBER"));
    nsmd_init_count = -1;
    return -1;
  } else if (nsmd_init_count > 0) {
    if (sys.ready > 0) {
      LOG("waiting for new MASTER");
      next = 30 * 100; // 30 sec
    } else {
      nsmd_init_count--;
      next = 75 + (int)(rand() * 75.0 / RAND_MAX); // ~1 sec (0.75 to 1.5 sec)
      LOG("standing for new master (%d) next=%d", nsmd_init_count, next);
    }
  } else if (sys.master < 0) { // nsmd_init_count == 0
    LOG("select myself as MASTER");
    nsmd_init_count = -1;
    if (! sys.con[NSMCON_UDP].timstart) {
      sys.con[NSMCON_UDP].timstart = sys.con[NSMCON_TCP].timstart;
    }
    sys.con[NSMCON_UDP].timevent = time(0);
    sys.master = NSMCON_TCP;
    sys.ready  = 3;  // ready by standing as a new master
    sys.generation++;
    return -1;
  } else if (sys.deputy >= 2) {
    return -1;
  } else {
    // if there's no deputy, look for it for ever
    next = 30 * 100; // 30 sec
  }

  // send a UDP broadcast packet
  nsmd_udpsend(0, NSMCMD_NEWDAEMON, NSM_PROTOCOL_VERSION, nsmd_priority);

  return next;
}
// -- nsmd_fmtcpy -------------------------------------------------------
//
// fmtcpy does partial copy of <len> bytes from <pos>
// from/to buf to/from datid
//
// Since dtfmt does not respect <pos>:<len>, it has to go through the
// entire dtfmt, but when <pos>:<len> is the end of previous call,
// one can skip fmt decoding up to what was done in the previous call.
// ------------------------------------------------------------------------
int
nsmd_fmtcpy(char* buf, int datid, int buftodat, int pos = 0, int len = 0,
            int* ichunkp = 0)
{
  NSMmem& mem = *nsmd_memp;
  NSMsys& sys = *nsmd_sysp;
  NSMdat& dat = sys.dat[datid];
  const char* fmt = dat.dtfmt;
  static char fmtc[] = "asid()";
  static char sizc[] = "124809"; // 0 for '(', 9 for ')'
  int loc = 0;
  const int chunksiz = NSM_UDPDATSIZ; // = 1464
  char* datp = MEMPTR(ntohl(dat.dtpos));
  char* src = buftodat ? buf : datp + pos;
  char* dst = buftodat ? datp + pos : buf;
  static int pos_cache = -1;
  static int dat_cache = -1;
  static int siz_cache = -1;
  static int num_cache = -1;
  static int dir_cache = -1;
  static const char* fmt_cache;
  int ichunk = 0;
  int boundary = 0;
  static int ichunk_cache = -1;
  static int boundary_cache = -1;

  /*
  DBG("fmtcpy datid %d buftodat %d pos %d len %d fmt %s %02x %02x %02x %02x",
      datid, buftodat, pos, len, fmt,
      (uint8_t)src[0], (uint8_t)src[1], (uint8_t)src[2], (uint8_t)src[3]);
  */

  if (len == 0 || len > chunksiz) len = chunksiz;
  if (pos == pos_cache && datid == dat_cache && buftodat == dir_cache) {
    fmt      = fmt_cache;
    ichunk   = ichunk_cache;
    boundary = boundary_cache;
    DBG("fmtcpy pos %d", pos);
  } else {
    fmt_cache = 0;
  }

  // loop over the fmt string
  int nest = 0;
  const char* fmtp_nest[16];
  int  repeat_nest[16];

  while (*fmt && *fmt != ' ') {
    const char* p = fmt_cache ? fmt_cache : strchr(fmtc, *fmt++);
    const int siz = fmt_cache ? siz_cache : sizc[p - fmtc] - '0';
    int       num = fmt_cache ? num_cache : isdigit(*fmt) ? atoi(fmt) : 1;
    if (! fmt_cache) { while (isdigit(*fmt)) fmt++; }

    if (siz == 0) {
      if (nest == 16) ASRT("nsmd_fmtcpy too many nested struct");
      repeat_nest[nest] = -1;
      fmtp_nest[nest] = fmt;
      nest++;
      continue;
    } else if (siz == 9) {
      if (nest <= 0) ASRT("nsmd_fmtcpy unexpected closing paren");
      if (repeat_nest[nest - 1] < 0) {
        repeat_nest[nest - 1] = num - 1;
      } else {
        repeat_nest[nest - 1]--;
      }
      if (repeat_nest[nest - 1] != 0) {
        fmt = fmtp_nest[nest - 1];
      } else {
        nest--;
      }
      continue;
    }

    // break if already reached the limit
    if (loc + siz > pos + len) break;

    // if pos is not at fmt type boundary, adjust it
    if (loc < pos && loc + siz * num > pos) {
      if ((pos - loc) % siz)
        ASRT("fmtcpy alignment dat %d pos %d %d %d", datid, pos, loc, siz);
      num -= (pos - loc) / siz;
      loc = pos;
    }

    // if length is exceeded, adjust num (and len)
    if (loc + siz * num > pos + len) {
      siz_cache = siz;
      num_cache = num - (pos + len - loc) / siz;
      num = (pos + len - loc) / siz;  // truncate
      len = pos - loc + num * siz;
    } else {
      siz_cache = 0;
      num_cache = 0;
    }

    // adjust and count boundaries
    while (pos + siz * num > boundary + chunksiz) {
      ichunk++;
      boundary = pos + siz * ((boundary + chunksiz - pos) / siz);
    }

    // if loc is, copy data
    if (loc >= pos) {
      for (int i = 0; i < num; i++) {
        switch (siz) {
          case 1: *(uint8_t*)dst = *(uint8_t*)src;         break;
          case 2: *(int16_t*)dst = ntohs(*(int16_t*)src);  break;
          case 4: *(int32_t*)dst = ntohl(*(int32_t*)src);  break;
          case 8: *(int64_t*)dst = ntohll(*(int64_t*)src); break;
        }
        src += siz;
        dst += siz;
      }
    }
    /*
    DBG("fmtcpy 3 loc=%d siz=%d num=%d pos=%d len=%d *fmt=%c",
    loc, siz, num, pos, len, *fmt);
    */
    loc += siz * num;
  }

  // cache
  pos_cache = loc;
  fmt_cache = fmt;
  ichunk_cache   = ichunk;
  boundary_cache = boundary;
  dir_cache = buftodat;
  if (ichunkp) *ichunkp = ichunk;

  // return copied length
  return loc - pos;
}
// -- nsmd_shmcast ------------------------------------------------------
void
nsmd_shmcast()
{
  NSMsys& sys = *nsmd_sysp;
  if (! nsmd_sysp) ASRT("shmcast: no sys");
  uint64_t now10ms = time10ms();

  for (int i = 0; i < sys.nsnd; i++) {
    NSMdat_snd& snd = sys.snd[i];
    NSMdat& dat = sys.dat[snd.disid];
    int dtcyc = ntohs(dat.dtcyc);

    if (now10ms < snd.distim - 2 * dtcyc) {
      // distim is quite in future and need to readjust,
      // maybe because the system clock has been externally adjusted
      snd.distim = now10ms + 2 * dtcyc;

    } else if (now10ms >= snd.distim) {
      int dtsiz = ntohs(dat.dtsiz);

      int offset = 0;
      const int chunksiz = NSM_UDPDATSIZ;
      uint64_t now1ms = time1ms();

      while (offset < dtsiz) {
        NSMudpbuf udpbuf;
        int siz = (dtsiz - offset) < chunksiz ? dtsiz - offset : chunksiz;
        int len = nsmd_fmtcpy(udpbuf.dat, snd.disid, 0, offset, siz);
        if (len <= 0) break;

        nsmd_udpsend(0, NSMCMD_USRCPYMEM, snd.disid, offset, &udpbuf, len);

        offset += len;
        uint64_t tt = time1ms();

        // QoS to keep 1 Mbyte/s or 1 packet / 1ms
        int dbgcnt = 0;
        while (now1ms + offset / 1000 > time1ms() && dbgcnt++ < 10000) {
          sleep1ms(1);
        }
        if (dbgcnt > 9000) {
          ASRT("shmcast dbgcnt");
        }
      }
      if (now10ms > snd.distim + dtcyc) {
        // distim is quite in past and need to readjust,
        // maybe because of system clock or process has been suspended
        snd.distim = now10ms + dtcyc;
      } else {
        snd.distim += dtcyc;
      }
    }
  }
}
// --            -- cmd service calls --
//               -----------------------
// -- nsmd_setup_daemon -------------------------------------------------
//
// - I'm master or deputy, and sys.master is always available
void
nsmd_setup_daemon(NSMcon& con, int exception)
{
  NSMsys& sys = *nsmd_sysp;
  NSMcon& master = sys.con[sys.master];
  NSMcon& deputy = sys.con[sys.deputy];
  NSMcon& tcpcon = sys.con[NSMCON_TCP];
  NSMcon& udpcon = sys.con[NSMCON_UDP];
  NSMdmsg dmsg;

  if (NoMaster()) ASRT("setup_daemon: no master");

  // send an answer
  LOG("setup_daemon M=%s/D=%s => %s",
      ADDR_STR(master.sockad),
      sys.deputy >= 0 ? ADDR_STR(deputy.sockad) : "(none)",
      &con == &tcpcon ? "everybody" : ADDR_STR(con.sockad));

  memset(&dmsg, 0, sizeof(dmsg));
  dmsg.src  = dmsg.dest = -1;
  dmsg.npar = 9;
  dmsg.pars[0] = ntohl(AddrMaster());
  dmsg.pars[1] = ntohl(NoDeputy() ? -1 : AddrDeputy());
  dmsg.pars[2] = hlltohl(master.timstart, 0);
  dmsg.pars[3] = hlltohl(master.timstart, 1);
  dmsg.pars[4] = NoDeputy() ? 0 : hlltohl(deputy.timstart, 0);
  dmsg.pars[5] = NoDeputy() ? 0 : hlltohl(deputy.timstart, 1);
  dmsg.pars[6] = hlltohl(udpcon.timstart, 0);
  dmsg.pars[7] = hlltohl(udpcon.timstart, 1);
  dmsg.pars[8] = sys.generation;

  if (&con < sys.con || &con >= sys.con + NSMSYS_MAX_CON) {
    NSMudpbuf udpbuf;
    memcpy(udpbuf.dat, dmsg.pars, sizeof(int32_t) * 9);
    nsmd_udpsend(&con.sockad, NSMCMD_ACKDAEMON, 0, 0, 0, 0,
                 9, dmsg.pars);

  } else if (&con == &tcpcon) {
    dmsg.req = NSMCMD_NEWMASTER;
    for (int conid = NSMCON_OUT; conid < sys.ncon; conid++) {
      if (conid == exception) continue;
      if (ConidIsLocal(conid)) continue;
      if (conid == sys.master || conid == sys.deputy) continue;
      nsmd_tcpsend(sys.con[conid], dmsg);
    }
  } else {
    dmsg.req = NSMCMD_NEWMASTER;
    nsmd_tcpsend(con, dmsg);
  }
}
// -- nsmd_touchsys -----------------------------------------------------
// new implementation to check and modify tcpq
//
// update partial NSMsys memory to "con" first, then to the rest of
// members by putting the message to "nsmd_tcpqfirst" for "con" unless
// sending to "con" is "inprogress".
//
// In reality, NSMsys update request from third node (such as by nsmd2
// initialization) may result in updating NSMsys earlier in elsewhere
// than "con".
//
// This may generate a case that third node recognizes the "con" node
// before "con" node is properly set up.  This makes an undelivered
// message.
// ----------------------------------------------------------------------
static void
nsmd_touchsys(NSMcon& con, uint32_t pos, uint16_t siz)
{
  NSMsys& sys = *nsmd_sysp;

  // parameter check
  if (siz == 0 || pos + siz > sizeof(sys)) {
    ASRT("touchsys pos/siz=%d/%d", pos, siz);
  }

  // I should be a master or a deputy when master is dying
  //if (! IamMaster()) ASRT("touchsys: I'm not master");
  if (! IamMaster() and !(IamDeputy() and ConIsMaster(con)))
    ASRT("touchsys: I'm not master or master is dying");

  // loop over tcpq
  int off = sizeof(NSMtcphead) + 2 * sizeof(int32_t); // always 2 params
  NSMDtcpq* q = nsmd_tcpqfirst;
  char* begp = (char*)&sys + pos;
  char* endp = begp + siz;
  NSMDtcpq* qptr[NSMSYS_MAX_CON];
  uint32_t qpos[NSMSYS_MAX_CON];
  uint16_t qsiz[NSMSYS_MAX_CON];
  memset(qptr, 0, sys.ncon * sizeof(NSMDtcpq*));
  memset(qpos, 0, sys.ncon * sizeof(uint32_t));
  memset(qsiz, 0, sys.ncon * sizeof(uint16_t));
  int conid = CON_ID(con);

  NSMDtcpq* inprogress = 0;
  while (q) {
    if (q->conid == conid && q->pos != 0) {
      inprogress = q;
      q = q->nextp;
      continue;
    } else if (q->conid == conid && inprogress) {
      q = q->nextp;
      continue;
    }
    if (q->req != NSMCMD_SYSCPYMEM) {
      q = q->nextp;
      continue;
    }
    const char* qbeg = q->datap;
    const char* qend = qbeg + q->msglen - off;
    if (endp < qbeg || qend < endp) {
      q = q->nextp;
      continue; // no change
    }
    if (qbeg < begp) {
      pos = SYSPOS(qbeg);
      siz += (begp - qbeg);
    }
    if (endp < qend) siz += (qend - endp);
    if (qptr[q->conid] || q->conid == conid) {
      q->req = NSMCMD_NOP;
    } else {
      qptr[q->conid] = q;
    }
    qpos[q->conid] = pos;
    qsiz[q->conid] = siz;
    q = q->nextp;
  }

  // set up most of dmsg
  NSMdmsg dmsg;
  static int seq = 0;
  memset(&dmsg, 0, sizeof(dmsg));
  dmsg.req = NSMCMD_SYSCPYMEM;
  dmsg.seq = ++seq;
  dmsg.src = dmsg.dest = -1;
  dmsg.npar = 2;
  dmsg.opt  = 0; // not for syscpy chain

  // loop over conid
  for (int i = NSMCON_OUT; i < sys.ncon; i++) {
    if (ConidIsLocal(i)) continue;

    // skip sending to removed or temporarily unavailable destination
    switch (sys.con[i].status) {
      case NSMDC_NA:
      case NSMDC_WA:
      case NSMDC_RM: continue;
    }

    dmsg.pars[0] = qpos[i] ? qpos[i] : pos;
    dmsg.pars[1] = (qsiz[i] ? qsiz[i] : siz) & 0xffff;
    dmsg.datap = SYSPTR(dmsg.pars[0]);
    dmsg.len   = dmsg.pars[1];

    if (i != conid) {
      // if qptr[i] == 0, added at the end
      nsmd_tcpsend(sys.con[i], dmsg, qptr[i], 0);
    } else if (inprogress) {
      nsmd_tcpsend(sys.con[i], dmsg, inprogress, 1);
    } else {
      nsmd_tcpsend(sys.con[i], dmsg, nsmd_tcpqfirst, -1);
    }
  }
}
// -- nsmd_touchsysq ----------------------------------------------------
// syscpyq based touchsys (old scheme)
// ----------------------------------------------------------------------
typedef struct nsmd_syscpyq {
  uint32_t ipaddr;
  int pos;
  int siz;
  struct nsmd_syscpyq* nextp;
} NSMDsyscpyq;
static NSMDsyscpyq nsmd_syscpytop = { 0, 0, 0, 0 };
static NSMDsyscpyq* nsmd_syscpylastp = &nsmd_syscpytop;
static void
nsmd_touchsysq(NSMcon& con, int pos, int siz)
{
  NSMsys& sys = *nsmd_sysp;

  ASRT("touchsysq"); // should be called

  if (! nsmd_syscpylastp) ASRT("touchsys: list is null");
  if (nsmd_syscpylastp->nextp) ASRT("touchsys: last->nextp is not null");
  if (! IamMaster() && ! ConIsMaster(con)) {
    WARN("touchsys: I'm not master and not for master");
    return;
  }

  // first, add for the connected node at the top
  if (ExistConid(CON_ID(con)) && ConIsLocal(con)) {
    NSMDsyscpyq* savep = nsmd_syscpytop.nextp;
    NSMDsyscpyq* nextp =
      (NSMDsyscpyq*)nsmd_malloc("touchsys", sizeof(NSMDsyscpyq));
    nsmd_syscpytop.nextp = nextp;
    nextp->pos = pos;
    nextp->siz = siz;
    nextp->ipaddr = ADDR_IP(con.sockad);
    nextp->nextp = savep;
    if (nsmd_syscpylastp == &nsmd_syscpytop)
      nsmd_syscpylastp = nsmd_syscpytop.nextp;
  }

  // then add all the other connected nodes at the end
  for (int conid = NSMCON_OUT; conid < sys.ncon; conid++) {
    if (CON_ID(con) == conid) continue;
    if (ConidIsLocal(conid)) continue;

    nsmd_syscpylastp->nextp
      = (NSMDsyscpyq*)nsmd_malloc("touchsys", sizeof(NSMDsyscpyq));
    nsmd_syscpylastp = nsmd_syscpylastp->nextp;
    NSMDsyscpyq* lastp = nsmd_syscpylastp;
    lastp->pos = pos;
    lastp->siz = siz;
    lastp->ipaddr = AddrConid(conid);
    lastp->nextp = 0;
  }
}
// -- nsmd_setup_client -------------------------------------------------
// only for a named client, not for an anonymous one
// ----------------------------------------------------------------------
int
nsmd_setup_client(NSMcon& con, uid_t uid, pid_t pid, const char* name)
{
  NSMsys& sys = *nsmd_sysp;
  NSMnod* nodlastp = sys.nod + NSMSYS_MAX_NOD;
  NSMnod* nodp = nodlastp;

  // check for duplication
  int hash = nsmlib_hash(&sys, sys.nodhash, NSMSYS_MAX_HASH, name, 0);
  if (hash >= 0) {
    nodp = (NSMnod*)SYSPTR(ntohl(sys.nodhash[hash]));
    if (strcmp(nodp->name, name) != 0) ASRT("wrong hash for %s", name);
    if (nodp->nodpid != 0) {
      LOG("NEWCLIENT: nodename %s is in use", name);
      return NSMENODEEXIST;
    }
  }

  // look for a free place, or if not found,
  // look for a tentatively-unused place
  NSMnod* nod2p = nodlastp;
  time_t tlast = time(0);
  if (nodp == nodlastp) {
    for (nodp = sys.nod; nodp < nodlastp; nodp++) {
      if (nodp->nodpid == 0) {
        if (nodp->name[0] == 0) break;
        time_t nodtim = ntohll(nodp->nodtim);
        if (nodtim < tlast) {
          nod2p = nodp;
          tlast = nodtim;
        }
      }
    }
  }
  if (nodp == nodlastp) nodp = nod2p;
  if (nodp == nodlastp) {
    WARN("NEWCLIENT: no more place for %s", name);
    return NSMEFULNODE;
  }

  // before setting new client, remove all data with this node
  // if still exist, because they must be invalid
  int nodeid = nodp - sys.nod;
  NSMnod& nod = *nodp;
  // nsmd_freeall(con, nodeid);

  nod.noddat = htonl(-1);
  nod.ipaddr = ADDR_IP(con.sockad);
  nod.nodpid = htonl(pid);
  nod.noduid = htonl(uid);
  nod.nodtim = htonll(time(0));
  memset(nod.name, 0, sizeof(nod.name));
  strncpy(nod.name, name, NSMSYS_NAME_SIZ);
  sys.nnod = htons(ntohs(sys.nnod) + 1);

  hash = nsmlib_hash(&sys, sys.nodhash, NSMSYS_MAX_HASH, nod.name, 1);
  DBG("setup_client hash=%d syspos=%d siz=%d",
      hash, SYSPOS(&sys.nodhash[hash]), sizeof(sys));

  nsmd_touchsys(con, SYSPOS(&nod), sizeof(nod));
  nsmd_touchsys(con, SYSPOS(&sys.nodhash[hash]), sizeof(int32_t));
  nsmd_touchsys(con, SYSPOS(&sys.nnod), sizeof(sys.nnod));

  return nodeid;
}
// -- nsmd_setup_req ----------------------------------------------------
// only for a named client, not for an anonymous one
// ----------------------------------------------------------------------
int
nsmd_setup_req(NSMcon& con, const char* name)
{
  NSMsys& sys = *nsmd_sysp;
  NSMreq* reqp = 0;
  NSMreq* reqlastp = sys.req + NSMSYS_MAX_REQ;

  // check for duplication
  int hash = nsmlib_hash(&sys, sys.reqhash, NSMSYS_MAX_HASH, name, 0);
  if (hash >= 0) {
    reqp = (NSMreq*)SYSPTR(ntohl(sys.reqhash[hash]));
    if (strcmp(reqp->name, name)) ASRT("setup_req wrong hash for %s", name);
    return ntohs(reqp->code);
  }

  // look for a free place
  if (sys.req[sys.nreq].code == 0) {
    reqp = &sys.req[sys.nreq];
  } else {
    for (reqp = sys.req; reqp < reqlastp; reqp++) {
      if (reqp->code == 0) break;
    }
  }
  sys.nreq++;
  strcpy(reqp->name, name);
  int reqcode = NSMREQ_FIRST + (reqp - sys.req);
  reqp->code = htons(reqcode);

  hash = nsmlib_hash(&sys, sys.reqhash, NSMSYS_MAX_HASH, reqp->name, 1);
  DBG("setup_req hash=%d for %s", hash, name);

  nsmd_touchsys(con, SYSPOS(reqp), sizeof(*reqp));
  nsmd_touchsys(con, SYSPOS(&sys.reqhash[hash]), sizeof(int32_t));

  return reqcode;
}
// -- nsmd_delete_req ---------------------------------------------------
// only for a named client, not for an anonymous one
// ----------------------------------------------------------------------
int
nsmd_delete_req(NSMcon& con, int reqcode)
{
  NSMsys& sys = *nsmd_sysp;
  int reqid = reqcode - NSMREQ_FIRST;
  NSMreq& req = sys.req[reqid];
  NSMreq* reqlastp = sys.req + NSMSYS_MAX_REQ;

  // check reqcode
  if (reqid < 0 || reqid >= NSMSYS_MAX_REQ || req.code != reqcode) {
    DBG("invalid reqcode %d", reqcode);
    return -1;
  }

  // get hash
  int hash = nsmlib_hash(&sys, sys.reqhash, NSMSYS_MAX_HASH, req.name, 0);
  if (hash < 0) {
    ASRT("delete_req no hash for %s", req.name);
    return -1;
  }

  // remove hash and code
  int nshift = nsmlib_hash(&sys, sys.reqhash, NSMSYS_MAX_HASH, req.name, -1);
  memset(req.name, 0, sizeof(req.name));
  req.code = -1;

  nsmd_touchsys(con, SYSPOS(&req), sizeof(req));
  if (hash + nshift > NSMSYS_MAX_HASH) {
    int a = NSMSYS_MAX_HASH - hash;
    int b = hash + nshift - NSMSYS_MAX_HASH;
    nsmd_touchsys(con, SYSPOS(&sys.reqhash[hash]), sizeof(int32_t) * a);
    nsmd_touchsys(con, SYSPOS(&sys.reqhash[0]),    sizeof(int32_t) * b);
  } else {
    nsmd_touchsys(con, SYSPOS(&sys.reqhash[hash]), sizeof(int32_t) * nshift);
  }

  return 0;
}
// -- nsmd_delete_dat ---------------------------------------------------
//
// ----------------------------------------------------------------------
void
nsmd_delete_dat(NSMcon& con, int datid)
{
  NSMsys& sys = *nsmd_sysp;
  NSMdat& dat = sys.dat[datid];
  int16_t* aprevp = &sys.afirst;
  int      anext = (int16_t)ntohs(*aprevp);
  int      nodid = (int16_t)ntohs(dat.owner);

  if (nodid == -1) {
    DBG("delete_dat datid=%d no owner", datid);
    return;
  }

  DBG("delete_dat datid=%d", datid);
  int16_t* nprevp = &sys.nod[nodid].noddat;
  int      nnext  = ntohs(*nprevp);

#if 0
  // look for nprevp in nodp
  for (int inod = 0; inod < nnod;) {
    DBG("delete_dat pid=%d noddat=%d", ntohl(nodp->nodpid), ntohs(nodp->noddat));
    if (nodp->nodpid != 0) {
      if (ntohs(nodp->noddat) == datid) {
        nprevp = &nodp->noddat;
        break;
      }
      inod++;
    }
  }
#endif

  // look for aprevp in anext chain
  while (anext != datid) {
    if (anext == -1) ASRT("delete_dat aprevp not found");
    aprevp = &sys.dat[anext].anext;
    anext  = ntohs(*aprevp);
  }
  while (nnext != datid) {
    if (nnext == -1) ASRT("delete_dat nprevp not found");
    nprevp = &sys.dat[nnext].nnext;
    nnext  = ntohs(*nprevp);
  }

#if 0
  // look for nprevp/aprevp in anext chain
  while (1) {
    if (anext == (uint16_t) - 1) {
      ASRT("delete_dat aprevp not found");
    }
    if (anext == datid) break;

    datp = sys.dat + anext;
    aprevp = &datp->anext;
    anext = ntohs(*aprevp);
    int nnext = ntohs(datp->nnext);
    if (nnext == datid) {
      DBG("delete_dat nnext=%d", nnext);
      nprevp = &datp->nnext;
    }
  }
#endif

  // reconnect nnext/anext chain
  if (! nprevp) ASRT("delete_dat no nprevp");
  *nprevp = dat.nnext;
  nsmd_touchsys(con, SYSPOS(nprevp), sizeof(*nprevp));

  // clear if not referred
  if (dat.dtref == 0) {
    if (! aprevp) ASRT("delete_dat no aprevp");
    *aprevp = dat.anext;
    nsmd_touchsys(con, SYSPOS(aprevp), sizeof(*aprevp));

    memset(dat.dtnam, 0, NSMSYS_DNAM_SIZ + 1);
    memset(dat.dtfmt, 0, NSMSYS_DFMT_SIZ + 1);
    dat.dtrev = 0;
    dat.dtpos = htonl(-1);
    dat.dtsiz = 0;
  }
  dat.owner = htons(-1);
  nsmd_touchsys(con, SYSPOS(&dat), sizeof(dat));
}
// -- nsmd_delete_nod ---------------------------------------------------
// delete node and related entry from master's distributed sys area
// ----------------------------------------------------------------------
void
nsmd_delete_nod(NSMcon& con, int nodid)
{
  NSMsys& sys = *nsmd_sysp;
  NSMnod& nod = sys.nod[nodid];
  int16_t n_nodid = (int16_t)htons(nodid);

  LOG("delete_nod con=%d id=%d mode=%d", &con - sys.con, nodid);

  // remove all my refs
  for (int refid = 0; refid < NSMSYS_MAX_REF; refid++) {
    NSMref& ref = sys.ref[refid];
    if (ref.refnod == n_nodid) {
      int datid = ntohs(ref.refdat);
      NSMdat& dat = sys.dat[datid];
      if (dat.dtref == 0) ASRT("delete_nod no dtref nod=%d dat=%d ref=%d",
                                 nodid, datid, refid);
      dat.dtref = htons(ntohs(dat.dtref) - 1);
      memset(&ref, 0, sizeof(ref));
      ref.refnod = ref.refdat = ntohs(-1);
      nsmd_touchsys(con, SYSPOS(&ref), sizeof(ref));
      nsmd_touchsys(con, SYSPOS(&dat.dtref), sizeof(dat.dtref));

      if (dat.dtref == 0 && dat.owner == -1) {
        LOG("delete_nod: removing %s (%d)", dat.dtnam, datid);
        nsmd_delete_dat(con, datid);
      } else {
        LOG("delete_nod: not removing %s (%d)", dat.dtnam, datid);
      }
    }
  }

  // remove all my dats
  int nnext = ntohs(nod.noddat);
  while (nnext != (uint16_t) - 1) {
    int datid = nnext;
    NSMdat& dat = sys.dat[datid];
    nnext = ntohs(dat.nnext);
    nsmd_delete_dat(con, datid);
  }
}
// -- nsmd_delete_dis ---------------------------------------------------
// delete node and related entry from local sys area
// ----------------------------------------------------------------------
void
nsmd_delete_dis(int nodid)
{
  NSMsys& sys = *nsmd_sysp;

  LOG("delete_dis nodid=%d", nodid);

  // remove all my disid
  for (int i = sys.nsnd - 1; i >= 0; i--) {
    if (sys.snd[i].disnod == nodid) {
      if (i != sys.nsnd - 1) {
        memcpy(&sys.snd[i], &sys.snd[i + 1], sizeof(sys.snd[0]) * (sys.nsnd - 1 - i));
      }
      sys.nsnd--;
      sys.snd[sys.nsnd].disnod = (uint16_t) - 1;
      sys.snd[sys.nsnd].disid  = (uint16_t) - 1;
    }
  }
}
// --                -- cmd calls --
//                   ---------------
// -- nsmlib_isvalidname ------------------------------------------------
#define IsInvalidSysSrcDest(src, dest, con) \
  ((NodeIsValid(src)   && dest != (uint16_t)-1) ||  \
   (NodeIsValid(dest)  && src  != (uint16_t)-1) || \
   (ConIsLocal(con)    && dest != (uint16_t)-1) || \
   (ConIsMaster(con)   && src  != (uint16_t)-1) || \
   (! ConIsMaster(con) && dest != (uint16_t)-1))

int
nsmlib_isvalidname(int len, const char* name, char* namebuf, int siz)
{
  int i;
  int isvalid = (len > siz) ? 0 : 1;
  memset(namebuf, 0, siz);
  for (i = 0; isvalid && i < len - 1 && i < siz; i++) {
    if (! isalnum(name[i]) && name[i] != '_') isvalid = 0;
    namebuf[i] = isprint(name[i]) ? name[i] : '?';
  }
  return isvalid;
}
// -- nsmd_tcpreply -----------------------------------------------------
// just to send one parameter back, mostly to send back an error
// ----------------------------------------------------------------------
void
nsmd_tcpreply(NSMcon& con, NSMdmsg& dmsg, int dest, int par)
{
  dmsg.npar = 1;
  dmsg.pars[0] = par;
  dmsg.src     = -1;
  dmsg.dest    = dest;
  dmsg.len     = 0;
  nsmd_tcpsend(con, dmsg);
}
// -- nsmd_do_killdaemon ------------------------------------------------
void
nsmd_do_killdaemon(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  if (dmsg.npar == 0) {
    nsmd_assert("killdaemon received from %s", ADDR_STR(con.sockad));
  } else {
    for (int conid = 2; conid < sys.ncon; conid++) {
      if (ADDR_IP(sys.con[conid].sockad) == dmsg.pars[0]) {
        LOG("killdaemon forwarding to %08x", ADDR_STR(sys.con[conid].sockad));
        dmsg.npar = 0;
        nsmd_tcpsend(sys.con[conid], dmsg);
        return;
      }
    }
    LOG("killdaemon forwarding to nowhere (%08x)", dmsg.pars[0]);
  }
}
// -- nsmd_do_newreq ----------------------------------------------------
//
// [both for master/deputy and non-master daemons]
//
// from local ---> request
//   if master ---> create and answer
//   else      ---> forward to master
// from master ---> answer to the local
// from non-master ---> what?
//
// (if anonymous, just locally accept)
// ----------------------------------------------------------------------
void
nsmd_do_newreq(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  // DBG("do_newreq");

  // dmsg should have at most one pars
  if (dmsg.npar != 0 && dmsg.npar != 1 || dmsg.len <= 1)
    ASRT("do_newreq invpar npar=%d len=%d", dmsg.npar, dmsg.len);

  // only either of source or destination nodeid should be there
  if (IsInvalidSysSrcDest(dmsg.src, dmsg.dest, con))
    ASRT("do_newreq internal error src=%d dest=%d conid=%d",
         dmsg.src, dmsg.dest, CON_ID(con));

  // master is needed
  if (sys.master == NSMCON_NON) {
    DBG("do_newreq no master");
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMASTER);
    return;
  }

  // name check before bothering master
  char namebuf[NSMSYS_NAME_SIZ + 1];
  int isvalid = nsmlib_isvalidname(dmsg.len, dmsg.datap, namebuf,
                                   NSMSYS_NAME_SIZ);
  if (! isvalid) { // unexpected because it's checked in nsmlib
    DBG("do_newreq invname len=%d %s", dmsg.len, namebuf);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVNAME);
    return;
  }

  // if from local and I'm not a master, forward it to the master
  if (ConIsLocal(con) && ! IamMaster()) {
    DBG("do_newreq forward to %d", sys.master);
    nsmd_tcpsend(sys.con[sys.master], dmsg);
    return;
  }

  // if I'm a master, set up the request
  if (IamMaster()) {
    int reqid = nsmd_setup_req(con, dmsg.datap);
    dmsg.npar = 1;
    dmsg.pars[0] = reqid;
    dmsg.dest = dmsg.src;
    dmsg.src  = -1;
    DBG("do_newreq reqid=%d", reqid);
    nsmd_tcpsend(con, dmsg);
    return;
  }

  // otherwise it should be from master to local
  if (! ConIsMaster(con)) ASRT("do_newreq internal error 1");
  if (dmsg.npar != 1) ASRT("do_newreq internal error 2 npar=%d", dmsg.npar);
  int conid = sys.conid[dmsg.dest];
  if (ExistConid(conid)) {
    nsmd_tcpsend(sys.con[conid], dmsg);
  } else {
    DBG("do_newreq client must have disappeared"); // but req is kept
  }
}
// -- nsmd_do_delreq ------------------------------------------------------
//
// [both for master/deputy and non-master daemons]
// ------------------------------------------------------------------------
void
nsmd_do_delreq(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  DBG("do_delreq");

  // dmsg should have at most one pars and no data
  if (dmsg.npar != 1 && dmsg.len != 0)
    ASRT("do_delreq invalid npar=%d len=%d", dmsg.npar, dmsg.len);

  // only either of source or destination nodeid should be there
  if (IsInvalidSysSrcDest(dmsg.src, dmsg.dest, con))
    ASRT("do_delreq invalid src=%d dest=%d conid=%d",
         dmsg.src, dmsg.dest, CON_ID(con));

  // master is needed
  if (sys.master == NSMCON_NON) {
    DBG("do_delreq no master");
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMASTER);
    return;
  }

  // if from local and I'm not a master, forward it to the master
  if (ConIsLocal(con) && ! IamMaster()) {
    DBG("do_delreq forward to %d", sys.master);
    nsmd_tcpsend(sys.con[sys.master], dmsg);
    return;
  }

  // if I'm a master, delete the request
  if (IamMaster()) {
    int ret = nsmd_delete_req(con, dmsg.pars[0]);
    dmsg.dest = dmsg.src;
    dmsg.src  = -1;
    DBG("do_delreq reqid=%d", dmsg.pars[0]);
    dmsg.pars[0] = ret;
    nsmd_tcpsend(con, dmsg);
    return;
  }

  // otherwise it should be from master to local
  if (! ConIsMaster(con)) ASRT("do_newreq internal error 1");
  int conid = sys.conid[dmsg.dest];
  if (ExistConid(conid)) {
    nsmd_tcpsend(sys.con[conid], dmsg);
  } else {
    DBG("do_delreq client must have disappeared"); // but req is removed
  }
}
// -- nsmd_do_newclient -------------------------------------------------
//
// [both for master/deputy and non-master daemons]
//
// from local ---> request
//   if master ---> create and answer
//   else      ---> forward to master
// from master ---> answer to the local
// from non-master ---> what?
//
// (if anonymous, just locally accept)
// ----------------------------------------------------------------------
void
nsmd_do_newclient(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;

  DBG("do_newclient");

  // msg should have two params (unexpected)
  if (dmsg.npar != 2) {
    WARN("newclient should have two pars, from %x", ADDR_STR(con.sockad));
    return;
  }
  // anonymous new client should be local (unexpected)
  if (dmsg.len <= 1 && ! ConIsLocal(con)) {
    WARN("newclient from %x should have a name", ADDR_STR(con.sockad));
    return;
  }

  // if from local, keep pid (and signal type) for later use
  if (ConIsLocal(con)) {
    time_t now = time(0);
    con.pid = dmsg.pars[1];
    con.timevent = now;
    con.timstart = now;
  }

  // if anonymous, return immediately without bothering master
  if (dmsg.len <= 1) {
    DBG("do_newclient anonymous");
    dmsg.pars[0] = con.nid = NSMSYS_MAX_NOD;
    nsmd_tcpsend(con, dmsg);
    return;
  }

  // name check before bothering master
  int isdirty = (dmsg.len > NSMSYS_NAME_SIZ) ? 1 : 0;
  const char* name = dmsg.datap;
  char namebuf[NSMSYS_NAME_SIZ + 1];
  memset(namebuf, 0, sizeof(namebuf));
  for (int i = 0; ! isdirty && i < dmsg.len - 1; i++) {
    if (! isalnum(name[i]) && name[i] != '_') isdirty++;
    namebuf[i] = isprint(name[i]) ? name[i] : '?';
  }
  if (isdirty) { // unexpected because it's checked in nsmlib
    DBG("do_newclient invname len=%d %s", dmsg.len, namebuf);
    dmsg.pars[0] = NSMEINVNAME;
    nsmd_tcpsend(con, dmsg);
    return;
  }

  // from here we need a master
  if (sys.master == NSMCON_NON) {
    DBG("do_newclient no master");
    dmsg.pars[0] = NSMENOMASTER;
    nsmd_tcpsend(con, dmsg);
    return;
  }

  // if I'm not master, new client from local is forwarded to master
  if (! IamMaster() && ConIsLocal(con)) {
    con.sigobs = (dmsg.req == NSMCMD_NEWCLIENT) ? 0 : 1;
    dmsg.req = NSMCMD_NEWCLIENT;
    DBG("do_newclient forward to %d", sys.master);
    nsmd_tcpsend(sys.con[sys.master], dmsg);
    return;
  }

  // set up new client if I'm master
  if (IamMaster()) {
    pid_t uid = dmsg.pars[0];
    uid_t pid = dmsg.pars[1];
    int nodeid = nsmd_setup_client(con, uid, pid, name);

    // reuse dmsg packet
    dmsg.pars[0] = nodeid;
    if (ConIsLocal(con) && nodeid >= 0) {
      con.nid = nodeid;
      sys.conid[nodeid] = &con - sys.con;
    }
    DBG("do_newclient immaster nodeid=%d", nodeid);
    nsmd_tcpsend(con, dmsg);
    return;
  }

  // otherwise it should be from master to local,
  // so send it back to the local process with the attached pid
  if (! ConIsMaster(con)) ERRO("do_newclient internal error 1");

  for (int conid = 2; conid < sys.ncon; conid++) {
    if (sys.con[conid].pid == dmsg.pars[1]) {
      if (! ConidIsLocal(conid)) ERRO("do_newclient internal error 2");
      if (dmsg.pars[0] < 0 || dmsg.pars[0] >= NSMSYS_MAX_NOD) {
        sys.con[conid].nid = -1;
      } else {
        sys.con[conid].nid = dmsg.pars[0];
        sys.conid[dmsg.pars[0]] = conid;
      }

      if (sys.con[conid].sigobs) dmsg.req = NSMCMD_NEWCLIENTOB;
      nsmd_tcpsend(sys.con[conid], dmsg);

      return;
    }
  }

  // delete client if it's already destroyed when a reply from master comes
  dmsg.req = NSMCMD_DELCLIENT;
  dmsg.src = dmsg.pars[0];
  dmsg.npar = 0;
  dmsg.len = 0;
  nsmd_tcpsend(con, dmsg); // con is master
}
// -- nsmd_do_delclient -------------------------------------------------
//
// [both for master/deputy and non-master daemons]
// - If master dies, do_delclient is called with ConIsMaster, and it's
//   deputy's task to remove all nodes on the master's host.
// ------------------------------------------------------------------------
void
nsmd_do_delclient(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;

  // basic checks
  if (sys.master < 0) {
    DBG("do_delclient no master?");
    return;
  }

  DBG("do_delclient");

  // free distribution list
  if (dmsg.src != (uint16_t) - 1 &&
      dmsg.dest == (uint16_t) - 1 &&
      ConIsLocal(con)) {
    nsmd_delete_dis(dmsg.src);
  }

  // I must be a master (or a deputy when con=master dies)
  if (! IamMaster() && !(IamDeputy() && ConIsMaster(con))) {
    DBG("do_delclient tcpsend");
    con.status = NSMDC_NA;
    sys.conid[con.nid] = 0;
    con.nid = -1;
    nsmd_tcpsend(sys.con[sys.master], dmsg); // forward to master
    return;
  }

  /*
  // I'm a master or I'm a deputy called by master
  if (dmsg.src >= 0 && dmsg.src < NSMSYS_MAX_NOD) {
    LOG("DELETE node=%s nodeid=%d", NODE_STR(dmsg.src), dmsg.src);
    NSMnod &nod = sys.nod[dmsg.src];
    nod.nodpid = 0; // everything else is kept
    nsmd_touchsys(con, SYSPOS(&nod.nodpid), sizeof(nod.nodpid));
  }*/

  // for anonymous
  if (con.nid == NSMSYS_MAX_NOD) {
    con.nid = -1;
    con.pid = 0;
    return;
  }

  // 1953 debug code
  if (dmsg.src != (uint16_t) - 1 && dmsg.dest != (uint16_t) - 1) {
    // send delclient to dest, not to src
    uint16_t tmp = dmsg.src;
    dmsg.src  = dmsg.dest;
    dmsg.dest = tmp;
  }

  // for an NSM client
  if (dmsg.src != (uint16_t) - 1) {
    if (dmsg.src == NSMSYS_MAX_NOD) return;
    if (dmsg.src >  NSMSYS_MAX_NOD) ASRT("do_delclient dmsg.src=%d", dmsg.src);
    NSMnod& nod = sys.nod[dmsg.src];

    /* 1953 debug -- this condition may need to be differently handled */
    if (nod.ipaddr == 0) {
      LOG("do_delclient: nod is already removed");
      return;
    }
    if (nod.ipaddr != ADDR_IP(con.sockad)) {
      LOG("do_delclient bad ip=%s", ADDR_STR(con.sockad));
      return;
    }

    /*
      IP address check was there in nsm1
     */

    // free dat area
    nsmd_delete_nod(con, dmsg.src);

    // clear nod entry
    LOG("delclient: nod=%s (%d)", nod.name, dmsg.src);
    // name is not cleared here
    nod.noddat = -1;
    nod.ipaddr = 0;
    nod.nodpid = 0;
    nod.noduid = 0;
    nod.nodtim = htonll(time(0));
    sys.nnod = htons(ntohs(sys.nnod) - 1);
    if (ConIsLocal(con)) {
      sys.conid[con.nid] = 0;
      con.nid = -1;
    }
    nsmd_touchsys(con, SYSPOS(&nod), sizeof(nod));
    nsmd_touchsys(con, SYSPOS(&sys.nnod), sizeof(sys.nnod));
    return;
  }

  // for an NSM daemon, delete all the clients
  if (dmsg.src != (uint16_t) - 1) ASRT("do_delclient invalid src=%d", dmsg.src);
  for (int nodid = 0; nodid < NSMSYS_MAX_NOD; nodid++) {
    NSMnod& nod = sys.nod[nodid];
    if (nod.ipaddr != ADDR_IP(con.sockad)) continue;
    dmsg.src = nodid;
    nsmd_do_delclient(con, dmsg);
  }
}
// -- nsmd_do_newdaemon -------------------------------------------------
//
// [most of part is only for master]
//
// - only for UDP before connection is established
// - received from newly started daemon (pars=[version,priority])
// ----------------------------------------------------------------------
void
nsmd_do_newdaemon(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;

  //DBG("do_newdaemon");

  // check if UDP
  if (con.sock != sys.con[NSMCON_UDP].sock) {
    ASRT("do_newdaemon %s is not UDP", ADDR_STR(con.sockad));
  }

  int version  = dmsg.pars[0];
  int priority = dmsg.pars[1];

  // check if somebody is running a different version of nsmd
  if (version != NSM_PROTOCOL_VERSION) {
    const char* comp = version > NSM_PROTOCOL_VERSION ? "newer" : "older";
    const char* verb = sys.ready > 0 ? "running" : "starting";
    LOG("nsmd of %s version (%d.%d.%02d) is %s at %s",
        comp, version / 1000, (version / 100) % 10, version % 100, verb,
        ADDR_STR(dmsg.from));
    if (version > NSM_PROTOCOL_VERSION) {
      nsmd_destroy(0);
    } else {
      // killing the other daemon is allowed even for non master/deputy
      nsmd_udpsend(&con.sockad, NSMCMD_NEWDAEMON,
                   NSM_PROTOCOL_VERSION, nsmd_priority);
    }
    return;
  }

  // answer the question if master is already established,
  if (sys.master > 0) {
    if (IamMaster() || IamDeputy()) {
      nsmd_setup_daemon(con);
    }
    // I'm not master/deputy, so it's not my job to answer the question
    return;
  }

  // more than one daemons are standing for master
  int32_t fromip = ADDR_IP(con.sockad);
  if (nsmd_init_count < 0) {
    ASRT("do we need to redo master search?");
    // below is just kept
    // somehow it's just before declaring master
    // but I'm not sure if this branch is reachable
    WARN("somehow redoing the master search");
    nsmd_init_count = NSMD_INITCOUNT_FIRST;
    nsmd_schedule(NSMCON_NON, NSMSCH_INITBCAST, 0, 0, 1);

  } else if (nsmd_priority < priority) {
    LOG("Other daemon with a lower ip address is starting up.");
    nsmd_init_count += NSMD_INITCOUNT_DELTA;

  } else if (nsmd_priority == priority && ntohl(fromip) < ntohl(AddrSelf())) {
    LOG("Other daemon with a lower ip address is starting up.");
    nsmd_init_count += NSMD_INITCOUNT_DELTA;

  }

  // broadcast message to me is to be ignored if I'm a better cadidate,
  // and the other node will wait until I'll be ready for the master,
  // since the other node will receive my broadcast with priority and ip.
  return;
}
// -- nsmd_do_usrcpymem -------------------------------------------------
void
nsmd_do_usrcpymem(NSMcon& con, NSMdmsg& dmsg)
{
  int datid  = dmsg.pars[0];
  int offset = dmsg.pars[1];
  uint32_t fromip = dmsg.from;
  NSMsys& sys = *nsmd_sysp;
  int owner = -1;
  int dtpos = -1;
  int dtsiz = 0;
  int badcpymem = 0;
  int nodip = 0;

  if (datid >= 0) {
    NSMdat& dat = sys.dat[datid];
    owner = (int32_t)ntohs(dat.owner);
    dtpos = (int32_t)ntohl(dat.dtpos);
    dtsiz = (int32_t)ntohs(dat.dtsiz);
  }

  if (owner == -1 || dtpos == -1 || dtsiz < dmsg.len + offset) {
    badcpymem = 1;
  } else {
    NSMnod& nod = sys.nod[owner];
    if (fromip != nod.ipaddr) badcpymem = 1;
    nodip = nod.ipaddr;
  }
  if (badcpymem) {
    static int nbadip = 0;
    static uint32_t badip[256];
    int i = 0;
    if (nbadip < lengthof(badip)) {
      for (i = 0; i < nbadip && fromip != badip[i]; i++)
        ;
      if (i == nbadip) {
        badip[nbadip++] = fromip;
        LOG("bad USRCPYMEM(dat=%d len=%d off=%d) from ip=%s (dbg owner=%d dtpos=%d dtsiz=%d fromip=%08x nodip=%08x)\n",
            datid, dmsg.len, offset, ADDR_STR(fromip),
            owner, dtpos, dtsiz, fromip, nodip
           );
      }
    }
  } else {
    nsmd_fmtcpy((char*)dmsg.datap, datid, 1, offset, dmsg.len);
  }
}
// -- nsmd_do_ackdaemon -------------------------------------------------
//
// [for non-master]
//
// - only for UDP before connection is established
// - received from master/deputy
// - pars[0] = master ip, pars[1] = deputy ip
//   pars[2,3] = master timstart, pars[4,5] = deputy timstart
//   pars[6,7] = nsm timstart, pars[8] = generation
// ----------------------------------------------------------------------
void
nsmd_do_ackdaemon(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;

  if (dmsg.npar != 9) ASRT("do_ackdaemon: dmsg.npar(%d) != 9", dmsg.npar);

  // check if UDP
  if (con.sock != sys.con[NSMCON_UDP].sock) {
    ASRT("do_ackdaemon %s is not UDP", ADDR_STR(con.sockad));
  }

  // check if anything to change
  int32_t newm = htonl(dmsg.pars[0]);  // converted back to network byte order
  int32_t newd = htonl(dmsg.pars[1]);
  int32_t oldm = sys.master ? AddrMaster() : -1;
  int32_t oldd = sys.deputy ? AddrDeputy() : -1;
  int32_t newgen = dmsg.pars[8];
  int32_t fromip = ADDR_IP(con.sockad);

  DBG("ACKDAEMON from %s m:%s=>%s d:%s=>%s g:%d=>%d",
      ADDR_STR(fromip),
      ADDR_STR(oldm),
      ADDR_STR(newm),
      ADDR_STR(oldd),
      ADDR_STR(newd),
      sys.generation,
      newgen);

  if (newm && newm == oldm && newd == oldd && newgen == sys.generation) return;

  // check the message source
  if (fromip != newm && fromip != newd) {
    WARN("ackdaemon from a non-master/deputy host %s", ADDR_STR(fromip));
    return;
  }

  // sys.ready
  if (sys.ready < 0) sys.ready = 0;

  // connect to master
  if (newm == nsmd_myip) {
    sys.master = NSMCON_TCP;
  } else if (newm == oldm) {
    // do nothing
  } else if (nsmd_newconn(sys.ncon, newm, "master") < 0) {
    // if it cannot connect, it may be because the master have
    // disappeared just now
    sys.master = NSMCON_NON;
    WARN("Cannot connect to master at %s", ADDR_STR(newm));
  } else {
    sys.master = sys.ncon++;
    sys.con[sys.master].timstart = hl2tohll(dmsg.pars[2], dmsg.pars[3]);
  }

  // connect to deputy if any
  if (! NoMaster() && newd != -1 && newd != nsmd_myip && oldd == -1) {
    if (nsmd_newconn(sys.ncon, newd, "deputy") < 0) {
      sys.deputy = NSMCON_NON;
      WARN("Cannot connect to deputy at %s", ADDR_STR(newd));
    } else {
      sys.deputy = sys.ncon++;
      sys.con[sys.deputy].timstart = hl2tohll(dmsg.pars[4], dmsg.pars[5]);
    }
  }

  // if cannot connect to master, find the master again.
  // if a new master is found, start copying memory
  if (sys.master == NSMCON_NON ||
      (sys.master == NSMCON_TCP && sys.deputy == NSMCON_NON)) {
    nsmd_init_count = NSMD_INITCOUNT_FIRST;
    nsmd_schedule(NSMCON_NON, NSMSCH_INITBCAST, 0, 0, 1);
  } else {
    sys.con[NSMCON_UDP].timstart = hl2tohll(dmsg.pars[6], dmsg.pars[7]);
    sys.generation = newgen;

    // call syscpymem to initiate REQCPYMEM cycle
    memset(&dmsg, 0, sizeof(dmsg));
    dmsg.req = NSMCMD_SYSCPYMEM;
    dmsg.npar = 2;
    dmsg.pars[0] = -1; // pos = -1
    dmsg.pars[1] = 0;  // siz = 0
    dmsg.opt     = 3;

    if (sys.master == NSMCON_TCP) {
      sys.generation++;
      nsmd_do_syscpymem(sys.con[sys.deputy], dmsg);
    } else {
      nsmd_do_syscpymem(sys.con[sys.master], dmsg);
    }
  }
}
// -- nsmd_do_syscpymem -------------------------------------------------
//
// - NSMCMD_SYSCPYMEM: pars=(pos,siz)
//
// - quadruple purpose
//   1. locally called from do_ackdaemon in a newly started nsmd, with
//      opt=3, npar=2: (pos,siz) = (-1,0)  to initiate REQCPY/SYSCPY chain
//      to start reqcpymem
//
//   2. called upon reqcpymem on master for sys memory
//      opt=1, npar=2: (pos,siz) for SYSPOS(pos) to SYSPOS(pos+siz)
//      to continue reqcpymem for 2., or for 3. at the end of sys memory,
//      or send ready if no dat to copy
//
//   3. called upon reqcpymem on master for mem memory
//      opt=2, npar=1: (datid)
//      to continue reqcpymem for 3., or send ready
//
//   4. called upon touchsys on master for sys memory
//      opt=0, npar=2: (pos,siz) for SYSPOS(pos) to SYSPOS(pos+siz)
//      nothing to continue
//
// - for syscpy chain: NSMCMD_REQCPYMEM: pars=(pos,siz)  (opt is not used)
// - for memcpy chain: NSMCMD_REQCPYMEM: pars=(datid)
// ----------------------------------------------------------------------
void
nsmd_do_syscpymem(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  int pos = dmsg.pars[0];
  int siz = dmsg.pars[1];
  int opt = dmsg.opt;
  static int seq = 0;
  const int tcpdatmax = NSM_TCPDATSIZ - 4 * sizeof(int32_t); // 64k - 16byte
  static int datid = -1;

  DBG("syscpymem opt=%d pos=%d siz=%d seq=%d<=%s",
      opt, pos, siz, dmsg.seq, ADDR_STR(con.sockad));
  /* ConIsMaster(con) ? " master" : ""); */

  if (! ConIsMaster(con) && !(ConIsDeputy(con) && IamMaster())) return;

  if (pos == -1 && siz == 0) {
    // called from ackdaemon to initiate REQCPYMEM cycle
    if (opt != 3) ASRT("do_syscpymem opt=%d, not from ackdaemon?", opt);
    pos = (char*)&sys.afirst - (char*)&sys;
    // BUG! pos = (char *)&sys.nod[0] - (char *)&sys;
    siz = tcpdatmax;
    datid = -1;
  } else {
    if (opt < 0 || opt > 2) ASRT("do_syscpymem opt=%d", opt);

    const char* typ = (opt == 2) ? "mem" : "sys";
    char*       ptr = (opt == 2) ? (char*)nsmd_memp : (char*)nsmd_sysp;
    int max = (opt == 2) ? sizeof(NSMmem) : (char*)&sys.con[0] - (char*)&sys;

    if (siz != dmsg.len) ASRT("%s wrong siz/len %d!=%d", typ, siz, dmsg.len);
    if (pos < 0)         ASRT("%s wrong pos=%d/%d", typ, pos, siz);
    if (pos + siz > max) ASRT("%s wrong pos=%d/%d", typ, pos, siz);

    // siz=0 if dat is already removed
    if (siz) memcpy(ptr + pos, dmsg.datap, siz);

    if (opt == 0) return; // done for simple copy

    // end of NSMsys, then switch to NSMmem
    if (opt == 1 && pos + siz == max) {
      opt = 2;
      datid = 0;
    } else if (opt == 1 && pos + siz > max) {
      ASRT("do_syscpymem pos=%d siz=%d > max=%d", pos, siz, max);
    }

    if (opt == 1) { // for NSMsys
      pos += siz;
      siz = tcpdatmax;
      if (pos + siz > max) siz = max - pos;
    } else { // for NSMmem
      for (; datid < NSMSYS_MAX_DAT; datid++) {
        NSMdat& dat = sys.dat[datid];
        if (dat.dtpos != -1 && dat.dtsiz != 0) break; // no need for ntohl/s
        if (dat.dtpos != -1 || dat.dtsiz != 0)
          ASRT("do_syscpymem broken dat=%d pos=%d siz=%d",
               datid, ntohl(dat.dtpos), ntohs(dat.dtsiz));
      }

      DBG("syscpymem datid=%d/%d", datid, NSMSYS_MAX_DAT);

      if (datid == NSMSYS_MAX_DAT) { // end of NSMmem
        memset(&dmsg, 0, sizeof(dmsg));
        dmsg.req = NSMCMD_READY;
        dmsg.opt = 1;
        dmsg.npar = 3;
        dmsg.pars[0] = nsmd_priority;
        dmsg.pars[1] = hlltohl(sys.con[NSMCON_TCP].timstart, 0);
        dmsg.pars[2] = hlltohl(sys.con[NSMCON_TCP].timstart, 1);
        DBG("syscpymem sending READY opt=%d prio=%d",
            dmsg.opt, nsmd_priority);
        if (IamMaster()) {
          sys.ready = 4;
        } else {
          sys.ready = 1;
        }
        if (! NoMaster() && ! IamMaster())
          nsmd_tcpsend(sys.con[sys.master], dmsg);
        if (! NoDeputy() && ! IamDeputy())
          nsmd_tcpsend(sys.con[sys.deputy], dmsg);
        return;
      }
      pos = datid++;
      siz = 0;
    }
  }

  memset(&dmsg, 0, sizeof(dmsg));
  dmsg.req = NSMCMD_REQCPYMEM;
  dmsg.seq = ++seq;
  dmsg.npar = siz ? 2 : 1;   // npar=2 for sys, npar=1/siz=0 for mem
  dmsg.pars[0] = pos;
  dmsg.pars[1] = siz;
  dmsg.len = 0;
  if (IamMaster()) {
    nsmd_tcpsend(sys.con[sys.deputy], dmsg);
  } else {
    nsmd_tcpsend(sys.con[sys.master], dmsg);
  }
}
// -- nsmd_do_reqcpymem -------------------------------------------------
//
// - NSMCMD_REQCPYMEM: pars=(pos,siz) for sys, pars=(datid) for mem
// - NSMCMD_SYSCPYMEM: pars=(pos,siz)
// ----------------------------------------------------------------------
void
nsmd_do_reqcpymem(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  static int seq = 0;
  int datid = -1;

  DBG("reqcpymem opt=%d pos=%d siz=%d",
      dmsg.npar == 2 ? 1 : 2, dmsg.pars[0], dmsg.pars[1]);

  // check if I'm master/deputy
  if (! IamMaster() && ! IamDeputy()) {
    if (sys.master < 0) ASRT("reqcpymem no master");
    WARN("reqcpymem I'm not master/deputy");
    dmsg.from = ADDR_IP(con.sockad);
    nsmd_tcpsend(sys.con[sys.master], dmsg); // forward
    return;
  }

  // parameters
  int opt = dmsg.npar == 2 ? 1 : 2; // npar=2 for sys, npar=1/siz=0 for mem
  int pos = dmsg.pars[0];
  int siz = dmsg.pars[1];
  const int tcpdatmax = NSM_TCPDATSIZ - 4 * sizeof(int32_t); // 64k - 16byte
  const char* typ = (opt == 2) ? "mem" : "sys";
  char*       ptr = (opt == 2) ? (char*)nsmd_memp : (char*)nsmd_sysp;

  if (opt == 2) {
    datid = pos;
    siz = (int16_t)ntohs(sys.dat[datid].dtsiz); // 0 if removed, but don't care
    pos = (int32_t)ntohl(sys.dat[datid].dtpos);
  }

  memset(&dmsg, 0, sizeof(dmsg));
  dmsg.req = NSMCMD_SYSCPYMEM;
  dmsg.seq = ++seq;
  dmsg.opt = opt;
  dmsg.npar = 2;
  dmsg.pars[0] = pos;
  dmsg.pars[1] = siz;
  dmsg.len = siz;
  dmsg.datap = pos >= 0 ? ptr + pos : 0;

  if (pos == -1 && siz == 0) {
    // this combination is to initiate cpymem cycle and
    // must be used only by do_ackdaemon function
    ASRT("do_reqcpymem pos=-1 siz=0 opt=%d datid=%d", opt, datid);
  }

  //DBG("reqcpymem syscpymem to %s", ADDR_STR(con.sockad));

  nsmd_tcpsend(con, dmsg);
}
// -- nsmd_do_ready -----------------------------------------------------
//
// - from newly started/reconnected daemon
// ----------------------------------------------------------------------
void
nsmd_do_ready(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  int olddeputy = NSMCON_NON;

  DBG("do_ready %d from conid=%d@%s",
      dmsg.opt, CON_ID(con), ADDR_STR(con.sockad));

  if (dmsg.npar != 3 && dmsg.npar != 0)
    ASRT("do_ready: dmsg.npar(%d) != 3 or 0", dmsg.npar);
  if (NoMaster()) return;

  int opt   = dmsg.opt; // 0: reconnection, 1:syscpymem, 2:newmaster
  int conid = &con - sys.con;
  int conip = ADDR_IP(con.sockad);
  //DBG("do_ready opt=%d prio=%d conid=%d", opt, prio, conid);

  con.ready = 1;

  if (dmsg.npar) {
    con.priority = dmsg.pars[0];
    con.timstart = hl2tohll(dmsg.pars[1], dmsg.pars[2]);
  }

  if (! IamMaster() && ! IamDeputy()) { // forward
    WARN("do_ready I'm not master/deputy");
    dmsg.from = conip;
    nsmd_tcpsend(sys.con[sys.master], dmsg); // forward
    return;
  }

  // opt == 0 when called from do_newmaster
  // opt == 1 when called from do_syscpymem

  if (opt == 0) return;

  // priority based rearrangement if needed
  int prio  = dmsg.pars[0];
  int changed = 1;

  if (IamDeputy()) { // ready after NSMCMD_SYSCPYMEM
    if (! ConIsMaster(con)) return;

    sys.generation++;

    if (prio < nsmd_priority ||
        (prio == nsmd_priority && ntohl(conip) > ntohl(nsmd_myip))) {

      ASRT("do_ready: this code probably doesn't work, so let's abort.");
      sys.master = NSMCON_TCP;
      sys.deputy = conid;
      DBG("new master(me), conid=%d prio=%d/%d ip=%08x/%08x", conid,
          nsmd_priority, prio, ntohl(nsmd_myip), ntohl(conip));
    } else {
      DBG("old master is ready again\n");
      changed = 0;
    }

  } else if (IamMaster()) { // ready after NSMCMD_SYSCPYMEM
    uint32_t masterip = ADDR_IP(sys.con[sys.master].sockad);
    uint32_t deputyip = ADDR_IP(sys.con[sys.deputy].sockad);
    int deputyprio = sys.con[sys.deputy].priority;
    if (prio > nsmd_priority ||
        (prio == nsmd_priority && ntohl(conip) < ntohl(masterip))) {
      sys.master = conid;
      olddeputy  = sys.deputy;
      sys.deputy = NSMCON_TCP;
      DBG("new master, conid=%d prio=%d/%d ip=%08x/%08x", conid,
          prio, nsmd_priority, ntohl(conip), ntohl(masterip));
    } else if (NoDeputy() || prio > deputyprio ||
               prio == deputyprio && ntohl(conip) < ntohl(deputyip)) {
      olddeputy  = sys.deputy;
      sys.deputy = conid;
      DBG("new deputy, conid=%d prio=%d/%d/%d ip=%08x/%08x/%08x", conid,
          prio, nsmd_priority, deputyprio,
          ntohl(conip), ntohl(masterip), ntohl(deputyip));
    } else if (ConIsDeputy(con)) {
      sys.generation++; // increment to let members reconnect
      DBG("old deputy is ready again\n");
      changed = 0;
    } else {
      DBG("new member, conid=%d prio=%d/%d/%d ip=%08x/%08x/%08x", conid,
          prio, nsmd_priority, deputyprio,
          ntohl(conip), ntohl(masterip), ntohl(deputyip));
      return;
    }
  }

  // If not reconnection, something must have changed to reach here, so
  // so send ACKDAEMON to relevant nodes
  if (changed) {
    nsmd_setup_daemon(con);
  }

  if (ConIsMaster(con) || ConIsDeputy(con)) {
    // send to olddeputy when deputy switched from olddeputy
    if (olddeputy != NSMCON_NON) {
      nsmd_setup_daemon(sys.con[olddeputy]);
    }

    // and send to everybody except master, deputy and olddeputy
    nsmd_setup_daemon(sys.con[NSMCON_TCP], olddeputy);
  }
}
// -- nsmd_do_newmaster -------------------------------------------------
//
// - newm==oldm, newd==oldd, newgen==oldgen (should not happen, redundant)
// - newm==oldm, newd==oldd, newgen!=oldgen (reconnection needed)
// - newm==oldm, oldd==-1   (first deputy)
// - newm==oldm, newd!=oldd (new deputy)
// - newm==oldd, newd==oldm (master<->deputy exchange)
// - newm!=oldd, newd==oldm (new master, deputy is old master)
// ----------------------------------------------------------------------
void
nsmd_do_newmaster(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;

  //DBG("do_newmaster");

  if (dmsg.npar != 9) ASRT("do_newmaster: dmsg.npar(%d) != 9", dmsg.npar);

  int oldm = NoMaster() ? -1 : ADDR_IP(sys.con[sys.master].sockad);
  int oldd = NoDeputy() ? -1 : ADDR_IP(sys.con[sys.deputy].sockad);
  int newm = htonl(dmsg.pars[0]);
  int newd = htonl(dmsg.pars[1]);
  int newgen = dmsg.pars[8];
  int oldgen = sys.generation;
  int itwasme = (oldm == nsmd_myip || oldd == nsmd_myip) ? 1 : 0;
  int itsme   = (newm == nsmd_myip || newd == nsmd_myip) ? 1 : 0;

  LOG("newmaster new=%s/%s old=%s/%s",
      ADDR_STR(newm), ADDR_STR(newd), ADDR_STR(oldm), ADDR_STR(oldd));

  int64_t tim = hl2tohll(dmsg.pars[6], dmsg.pars[7]);
  if (tim != sys.con[NSMCON_UDP].timstart) {
    ASRT("do_newmaster: wrong tim=%lx/%lx", tim, sys.con[NSMCON_UDP].timstart);
  }
  if (abs(newgen - oldgen) > 1) {
    ASRT("do_newmaster: generation leap %d -> %d", oldgen, newgen);
  }

  // -- check if unchanged
  if (newm == oldm && newd == oldd && newgen == oldgen) {
    LOG("do_newmaster: no change");
    return;
  }

  sys.generation = newgen;

  // -- check typical errorneous combinations
  if (newm == newd) ASRT("do_newmaster: pars=%x/%x", newm, newd);
  if (! newm || newm == -1) ASRT("do_newmaster: invalid newm=%x", newm);
  if (! newd || newd == -1) ASRT("do_newmaster: invalid newd=%x", newd);
  if (newm != oldm && newm != oldd && newd != oldm && newd != oldd) {
    ASRT("do_newmaster: too many changes %08x %08x => %08x %08x",
         oldm, oldd, newm, newd);
  }

  // -- check if disconnection is needed
  if (! itsme) {
    if (itwasme) {
      for (int conid = sys.ncon - 1; conid >= NSMCON_OUT; conid--) {
        if (ConidIsLocal(conid)) continue;
        int conip = AddrConid(conid);
        if (conip != newm && conip != newd) {
          nsmd_destroyconn(sys.con[conid], 0, "newmaster");
        }
      }
    } else {
      if (oldd != NSMCON_NON && oldd != newd && oldd != newm) {
        nsmd_delconn(sys.con[sys.deputy]);
        sys.deputy = NSMCON_NON;
      }
      if (oldm != NSMCON_NON && oldm != newm && oldm != newd) {
        nsmd_delconn(sys.con[sys.master]);
        sys.master = NSMCON_NON;
      }
      if (newgen != oldgen && ConIsMaster(con) && oldd != -1 &&
          sys.deputy != NSMCON_NON) {
        nsmd_delconn(sys.con[sys.deputy]);
        sys.deputy = NSMCON_NON;
      }
      if (newgen != oldgen && ConIsDeputy(con) && oldm != -1 &&
          sys.master != NSMCON_NON) {
        nsmd_delconn(sys.con[sys.master]);
        sys.master = NSMCON_NON;
      }
    }
  }

  // -- swap master and deputy
  if (newm == oldd || newd == oldm) {
    int tmp    = sys.deputy;
    sys.deputy = sys.master;
    sys.master = tmp;
    if (newm != oldd) sys.master = NSMCON_NON;
    if (newd != oldm) sys.deputy = NSMCON_NON;
  } else {
    if (newm != oldm) sys.master = NSMCON_NON;
    if (newd != oldd) sys.deputy = NSMCON_NON;
  }

  // -- reassign if found in existing con
  for (int conid = NSMCON_OUT; conid < sys.ncon; conid++) {
    if (sys.master != NSMCON_NON && sys.deputy != NSMCON_NON) break;
    if (ConidIsLocal(conid)) continue;
    int conip = AddrConid(conid);
    if (sys.master == NSMCON_NON && conip == newm) sys.master = conid;
    if (sys.deputy == NSMCON_NON && conip == newd) sys.deputy = conid;
  }

  // -- connect to missing master and deputy
  int cons_to_send_ready[2];
  cons_to_send_ready[0] = NSMCON_NON;
  cons_to_send_ready[1] = NSMCON_NON;

  if (newm == nsmd_myip && sys.master != NSMCON_TCP) {
    sys.con[NSMCON_UDP].timevent = time(0);
    sys.master = NSMCON_TCP;
  } else if (sys.master == NSMCON_NON) {
    if (nsmd_newconn(sys.ncon, newm, "MASTER") < 0) {
      ASRT("do_newmaster: failed to connect to master=%08x", newm);
    }
    sys.master = sys.ncon++;
    sys.con[sys.master].timstart = hl2tohll(dmsg.pars[2], dmsg.pars[3]);
    cons_to_send_ready[0] = sys.master;
  }

  if (newd == nsmd_myip && sys.deputy != NSMCON_TCP) {
    sys.con[NSMCON_UDP].timevent = time(0);
    sys.deputy = NSMCON_TCP;
  } else if (sys.deputy == NSMCON_NON) {
    if (nsmd_newconn(sys.ncon, newd, "DEPUTY") < 0) {
      ASRT("do_newmaster: failed to connect to deputy=%08x", newd);
    }
    sys.deputy = sys.ncon++;
    sys.con[sys.deputy].timstart = hl2tohll(dmsg.pars[4], dmsg.pars[5]);
    cons_to_send_ready[1] = sys.deputy;
  }

  // -- send ready
  for (int i = 0; i < 2; i++) {
    int conid = cons_to_send_ready[i];
    if (conid != NSMCON_NON) {
      memset(&dmsg, 0, sizeof(dmsg));
      dmsg.req = NSMCMD_READY;
      dmsg.npar = 3;
      // TODO: opt to nonzero when connection error handling is implemented
      dmsg.opt  = 0;
      dmsg.pars[0] = nsmd_priority;
      dmsg.pars[1] = hlltohl(sys.con[NSMCON_UDP].timstart, 0);
      dmsg.pars[2] = hlltohl(sys.con[NSMCON_UDP].timstart, 1);
      nsmd_tcpsend(sys.con[conid], dmsg);
    }
  }
  sys.ready = 2;
}
// -- nsmd_sysmsgchk ----------------------------------------------------
int
nsmd_sysmsgchk(const char* func, NSMcon& con, NSMdmsg& dmsg,
               int npar_min, int npar_max, int len_min, int len_max)
{
  NSMsys& sys = *nsmd_sysp;

  if (npar_min <= npar_max &&
      (dmsg.npar < npar_min || dmsg.npar > npar_max) &&
      (!(dmsg.npar == 1 && dmsg.pars[0] < 0)))
    ASRT("%s invalid npar=%d not in [%d,%d]",
         func, dmsg.npar, npar_min, npar_max);

  if (len_min <= len_max && (dmsg.len < len_min || dmsg.len > len_max))
    ASRT("%s invalid nlen=%d not in [%d,%d]",
         func, dmsg.len, len_min, len_max);

  // only either of source of destination nodeid should be there
  if (IsInvalidSysSrcDest(dmsg.src, dmsg.dest, con))
    ASRT("%s invalid src=%d dest=%d conid=%d",
         func, dmsg.src, dmsg.dest, CON_ID(con));

  // master is needed
  if (sys.master == NSMCON_NON) {
    DBG("%s no master", func);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMASTER);
    return -1;
  }

  // either to master or from master
  if (!(IamMaster() || ConIsLocal(con) || ConIsMaster(con)))
    ASRT("%s not from/to master", func);

  return 0;
}
// -- nsmd_fmtcheck -----------------------------------------------------
int
nsmd_fmtcheck(const char* fmtname, const char** fmtp = 0)
{
  int fmtsize = 0;
  for (const char* p = fmtname; *p && *p != ' '; p++) {
    int siz = 0;
    switch (p[0]) {
      case '(': siz = nsmd_fmtcheck(p + 1, &p); break;
      case 'a': siz = 1; break;
      case 's': siz = 2; break;
      case 'i': siz = 4; break;
      case 'd': siz = 8; break;
      case ')':
        if (fmtp) *fmtp = p;
        return fmtsize;
      default:
        return 0;
    }
    int num = 1;
    if (isdigit(p[1])) num = atoi(p + 1);
    if (num <= 0) return 0;
    while (isdigit(p[1])) p++;
    fmtsize += siz * num;
  }
  return fmtsize;
}
// -- nsmd_do_openmem ---------------------------------------------------
//
// request: (from client to master or remote to master)
//   pars[0] = datid
// return to local:
//   pars[0] >= 0 (datid, redundant) or < 0 (error)
// ----------------------------------------------------------------------
void
nsmd_do_openmem(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  NSMdat& dat = sys.dat[dmsg.pars[0]];
  int n_datid = htons(dmsg.pars[0]);
  int n_nodid = htons(dmsg.src);

  DBG("do_openmem datid=%d nodid=%d", dmsg.pars[0], dmsg.src);

  if (nsmd_sysmsgchk("do_openmem", con, dmsg, 1, 1, 0, 0) < 0)
    return;

  DBG("do_openmem 2");

  // if it is from master, send it back to client
  if (ConIsMaster(con)) {
    int conid = sys.conid[dmsg.dest];
    if (ExistConid(conid)) {
      DBG("do_openmem send it back to client conid=%d dest=%d", conid, dmsg.dest);
      nsmd_tcpsend(sys.con[conid], dmsg);
    } else {
      LOG("do_openmem client must have disappeared");
    }
    return;
  }

  // if I'not a master, forward it to the master
  if (! IamMaster()) {
    DBG("do_openmem forward to master");
    nsmd_tcpsend(sys.con[sys.master], dmsg);
    return;
  }

  // loop over ref
  int refempty = -1;
  int refid;
  for (refid = 0; refid < NSMSYS_MAX_REF; refid++) {
    NSMref& ref = sys.ref[refid];
    if (ref.refnod == -1 && refempty == -1) refempty = refid;
    if (ref.refnod == n_nodid && ref.refdat == n_datid) break;
  }
  if (refid != NSMSYS_MAX_REF) {
    DBG("do_openmem already opened");
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEOPENED);
    return;
  }
  if (refempty == -1) {
    DBG("do_openmem no empty space");
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMOREMEM);
    return;
  }

  // setup ref
  NSMref& ref = sys.ref[refempty];
  ref.refnod = n_nodid;
  ref.refdat = n_datid;
  ref.reftim = htonll(time(0));
  nsmd_touchsys(con, SYSPOS(&ref), sizeof(ref));

  dat.dtref = htons(ntohs(dat.dtref) + 1);
  nsmd_touchsys(con, SYSPOS(&dat.dtref), sizeof(dat.dtref));

  sys.nref = htons(ntohs(sys.nref) + 1);
  nsmd_touchsys(con, SYSPOS(&sys.nref), sizeof(sys.nref));

  DBG("do_openmem at %x", ntohl(dat.dtpos));
  nsmd_tcpreply(con, dmsg, dmsg.src, dmsg.pars[0]);
}
// -- nsmd_sendreq ------------------------------------------------------
void
nsmd_sendreq(NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;

  if (dmsg.dest == (uint16_t) - 1) {
    WARN("wrong destination (-1) from src=%d req=%04x", dmsg.src, dmsg.req);
    return;
  }

  // check the cache
  NSMnod& nod = sys.nod[dmsg.dest];
  int conid = sys.conid[dmsg.dest];
  if (conid) {
    if (nod.ipaddr == nsmd_myip) {
      if (sys.con[conid].nid != dmsg.dest) {
        conid = 0;
      }
    } else {
      if (ADDR_IP(sys.con[conid].sockad) == nsmd_myip) conid = 0;
      if (! IamMaster() && ! IamDeputy() && conid != sys.master) conid = 0;
    }
  }

  if (conid == 0) {
    if (nod.ipaddr == nsmd_myip) {
      for (int i = NSMCON_OUT; i < sys.ncon && conid == 0; i++) {
        if (sys.con[i].nid == dmsg.dest) conid = i;
      }
    } else if (IamMaster() || IamDeputy()) {
      for (int i = NSMCON_OUT; i < sys.ncon && conid == 0; i++) {
        if (ADDR_IP(sys.con[i].sockad) == nod.ipaddr) conid = i;
      }
    } else {
      conid = sys.master;
    }
  }

  char databuf[80];
  if (dmsg.datap) {
    int i = 0;
    while (isprint(dmsg.datap[i]) && i < sizeof(databuf) - 1) {
      databuf[i] = dmsg.datap[i];
      i++;
    }
    databuf[i] = 0;
  } else {
    strcpy(databuf, "(null)");
  }

  if (conid != 0) {
    DBG("command: req = %04x len = %d npar = %d datap = %s con = %d",
        dmsg.req, dmsg.len, dmsg.npar, databuf, conid);
    nsmd_tcpsend(sys.con[conid], dmsg);
  } else {
    DBG("ignored: req = %04x len = %d npar = %d datap = %s",
        dmsg.req, dmsg.len, dmsg.npar, databuf);
  }
  return;
}
// -- nsmd_do_allocmem --------------------------------------------------
//
// request: (from client to master or remote to master)
//   pars[0] = freq, pars[1] = revision, datap = "name fmt";
// return to local:
//   pars[0] >= 0 (datid) or < 0 (error), datap = 0
// ----------------------------------------------------------------------
void
nsmd_do_allocmem(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  NSMmem& mem = *nsmd_memp;
  uint64_t now10ms = time10ms();
  int      datid;
  DBG("do_allocmem");

  if (nsmd_sysmsgchk("do_allocmem", con, dmsg, 1, 2, 0, -1) < 0)
    return;

  // if it is from master, datap should be 0
  if (ConIsMaster(con)) {
    if (dmsg.len > 0) ASRT("do_allocmem non-zero data from master");
    int conid = sys.conid[dmsg.dest];
    if (ExistConid(conid)) {
      nsmd_tcpsend(sys.con[conid], dmsg);
      datid = dmsg.pars[0];
      if (datid >= 0) {
        for (int i = 0; i < sys.nsnd; i++) {
          if (sys.snd[i].disid == datid) ASRT("do_allocmem existing disid");
        }
        if (sys.nsnd >= NSMSYS_MAX_DAT) ASRT("do_allocmem too many disid");
        NSMdat_snd& snd = sys.snd[sys.nsnd++];
        memset(&snd, 0, sizeof(snd));
        snd.disid  = datid;
        snd.disnod = dmsg.dest;
        snd.distim = now10ms;
      }
    } else {
      DBG("do_allocmem client must have disappeared");
    }
    return;
  }

  // otherwise datap should have three strings
  const char* p = dmsg.datap ? strchr(dmsg.datap, ' ') : 0;
  int len  = p ? p - dmsg.datap : 0;
  if (dmsg.len < 3 || ! p || len >= NSMSYS_DNAM_SIZ ||
      dmsg.len - len - 1 >= NSMSYS_DFMT_SIZ) {
    DBG("do_allocmem wrong len(%d) or datap(%x,%d)", dmsg.len, p, len);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVPAR);
    return;
  }

  // check if the name is valid
  char datname[NSMSYS_DNAM_SIZ + 1];
  if (! nsmlib_isvalidname(len + 1, dmsg.datap, datname, NSMSYS_DNAM_SIZ + 1)) {
    DBG("do_allocmem bad name(%s)", datname);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVNAME);
    return;
  }

  // check the format
  char fmtstr[NSMSYS_DNAM_SIZ + 1];
  strncpy(fmtstr, p + 1, dmsg.len - len - 1);
  fmtstr[dmsg.len - len - 1] = 0;
  char* fmtname = strchr(fmtstr, ' ');
  if (fmtname) *fmtname++ = 0;
  int fmtsize = nsmd_fmtcheck(fmtstr, 0);
  if (fmtsize <= 0 || fmtsize > NSM_DATMAXSIZ) {
    DBG("do_allocmem bad fmt(siz=%d)", fmtsize);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVFMT);
    return;
  }
  DBG("allocmem p+1 %s", p + 1);

  // check the revision
  if (dmsg.npar != 2 || dmsg.pars[1] <= 0) {
    DBG("do_allocmem bad revision(npar=%d siz=%d)", dmsg.npar,
        dmsg.npar == 2 ? dmsg.pars[1] : 0);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVPAR);
    return;
  }
  int revision = dmsg.pars[1];

  // if I'not a master, forward it to the master
  if (! IamMaster()) {
    DBG("do_allocmem forward to master");
    nsmd_tcpsend(sys.con[sys.master], dmsg);
    return;
  }

  // find the end of datp for this nodeid
  NSMnod& nod = sys.nod[dmsg.src];
  NSMdat* nodeap = 0;
  if ((datid = ntohs(nod.noddat)) != (uint16_t) - 1) {
    do {
      nodeap = sys.dat + datid;
    } while ((datid = ntohs(nodeap->nnext)) != (uint16_t) - 1);
  }
  int16_t* nprevp = nodeap ? &nodeap->nnext : &nod.noddat;

  // go through the link to find a new or already allocated space
  uint16_t afirst = ntohs(sys.afirst);
  NSMdat* datp  = (afirst == (uint16_t) - 1) ? 0 : sys.dat + afirst;
  NSMdat* datp_sav   = 0;
  NSMdat* datp_aprev = 0; // one datp before the new space
  int prevend = MEMPOS(mem.mem);
  int previd = -1;
  int newdat = -1;
  int newpos = -1;
  int newpad = -1; // wasting pad size, to be minimized
  int existed = 0;

  DBG("do_allocmem: prevend = %d\n", prevend);

  while (datp) {

    // check if there is a space
    int space = ntohl(datp->dtpos) - prevend;
    if (space >= fmtsize && newpad >= 0 && newpad > space - fmtsize) {
      newpos = prevend;
      newpad = space - fmtsize;
      datp_aprev = datp_sav;
    }

    // check if the same datname exists
    int anext = ntohs(datp->anext);
    if (strcmp(datname, datp->dtnam) == 0) {
      if (datp->owner != -1) {
        LOG("allocmem: %s already owned by %d", datname, datp->owner);
        nsmd_tcpreply(con, dmsg, dmsg.src, NSMEMEMEXIST);
        return;
      } else if (revision > ntohs(datp->dtrev)) {
        /*
          New revision!
          Has to kill all the client which uses the old revision!
         */

        // loop over ref
        int refid;
        int n_datid = htons(datp - sys.dat);
        for (refid = 0; refid < NSMSYS_MAX_REF; refid++) {
          NSMref& ref = sys.ref[refid];
          if (ref.refdat == n_datid) {
            int node = ntohs(ref.refnod);

            NSMdmsg dmsg;
            memset(&dmsg, 0, sizeof(dmsg));
            dmsg.req = NSMCMD_DELCLIENT;
            dmsg.src = -1;
            dmsg.dest = node;
            nsmd_sendreq(dmsg);
          }
        }

      } else if (revision < ntohs(datp->dtrev)) {
        /* Old revision!  Has to fail */
        nsmd_tcpreply(con, dmsg, dmsg.src, NSMEMEMEXIST);
        return;
      } else if (strcmp(fmtstr, datp->dtfmt) != 0) {
        /* Different format with the same revision!  Has to fail. */
        nsmd_tcpreply(con, dmsg, dmsg.src, NSMEMEMEXIST);
        return;
      }

      // if exist, update
      existed = 1;
      newpos = ntohl(datp->dtpos);
      DBG("do_allocmem: newpos(dtpos) = %d\n", newpos);
      break;
      /*
      *nprevp = htons(datp - sys.dat);
      nsmd_touchsys(con, SYSPOS(nprevp), sizeof(int16_t));

      datp->owner = htonl(&nod - sys.nod);
      datp->dtcyc = htons(dmsg.pars[0] ? dmsg.pars[0] : 3000);
      nsmd_touchsys(con, SYSPOS(datp), sizeof(*datp));
      return;  // ここで return したら client が piperead できない!
      */
    }

    prevend = ntohl(datp->dtpos) + ntohs(datp->dtsiz);
    DBG("do_allocmem: prevend(dtpos+dtsiz) = %d\n", prevend);
    datp_sav = datp;

    if (ntohs(datp->anext) == (uint16_t) - 1) break;

    datp = sys.dat + ntohs(datp->anext);
  }

  // if space not found, there must be space after the last one
  if (! existed) {
    if (! datp_aprev) {
      if (prevend + fmtsize > NSM2_MEMSIZ) {
        LOG("allocmem: no more memory, cannot find %d bytes (prevend=%d)",
            fmtsize, prevend);
        nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMOREMEM);
        return;
      }
      datp_aprev = datp_sav; // this may be zero if no other dat
      newpos = prevend;
      DBG("do_allocmem: newpos(prevend) = %d\n", prevend);
    }

    // look for a free sys.dat entry to store
    for (datp = sys.dat; datp < sys.dat + NSMSYS_MAX_DAT; datp++) {
      if (datp->dtsiz == 0) break;
    }
    if (datp == sys.dat + NSMSYS_MAX_DAT) {
      LOG("allocmem: no more memory, cannot find %d bytes", fmtsize);
      nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMOREMEM);
      return;
    }

    // update anext chain
    if (datp_aprev) {
      datp->anext = datp_aprev->anext;
      datp_aprev->anext = ntohs(datp - sys.dat);
      nsmd_touchsys(con, SYSPOS(&datp_aprev->anext), sizeof(int16_t));
    } else {
      sys.afirst  = ntohs(datp - sys.dat);
      datp->anext = ntohs(-1);
      nsmd_touchsys(con, SYSPOS(&sys.afirst), sizeof(int16_t));
    }
  }

  // update nnext chain and other datp entries
  datid = datp - sys.dat;
  *nprevp = htons(datid);
  nsmd_touchsys(con, SYSPOS(nprevp), sizeof(int16_t));

  datp->nnext = htons(-1);
  datp->owner = htons(&nod - sys.nod);
  datp->dtpos = htonl(newpos);

  DBG("do_allocmem: datp(dtpos) = %d\n", newpos);

  datp->dtsiz = htons(fmtsize);
  datp->dtcyc = htons(dmsg.pars[0] ? dmsg.pars[0] : 1000); // 10 second
  if (! existed) datp->dtref = htons(0);
  datp->dtrev = htons(revision);
  strcpy(datp->dtnam, datname);
  strcpy(datp->dtfmt, fmtstr);
  if (fmtname) strcpy(datp->dtfmt + strlen(fmtstr) + 1, fmtname);
  nsmd_touchsys(con, SYSPOS(datp), sizeof(*datp));

  // return if it is from outside
  LOG("allocmem: %s %d bytes at %d (datid=%d)",
      datname, fmtsize, newpos, datid);
  nsmd_tcpreply(con, dmsg, dmsg.src, datid);
  if (! ConIsLocal(con)) return;

  // if it is the memory of this node, initialize datsnd buffer
  for (int i = 0; i < sys.nsnd; i++) {
    if (sys.snd[i].disid == datid) ASRT("do_allocmem existing disid");
  }
  if (sys.nsnd >= NSMSYS_MAX_DAT) ASRT("do_allocmem too many disid");
  NSMdat_snd& snd = sys.snd[sys.nsnd++];
  memset(&snd, 0, sizeof(snd));
  snd.disid  = datid;
  snd.disnod = &nod - sys.nod;
  snd.distim = now10ms;
}
// -- nsmd_do_flushmem --------------------------------------------------
//
// request: (from client to nsmd)
//   pars[0] = ppos, pars[1] = psiz, datap = 0
// return to local client:
//   pars[0] >= 0 (datid) or < 0 (error), datap = 0
// ----------------------------------------------------------------------
void
nsmd_do_flushmem(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  NSMmem& mem = *nsmd_memp;
  NSMudpbuf udpbuf;
  int ppos = dmsg.pars[0];
  int psiz = dmsg.pars[1];
  int dpos = 0;
  int dsiz = 0;
  int n = 0;

  if (nsmd_sysmsgchk("do_flushmem", con, dmsg, 2, 2, 0, 0) < 0 ||
      dmsg.src == (uint16_t) - 1) {
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVPAR);
    return;
  }

  if (psiz < 0 || ppos < 0 || ppos >= NSM2_MEMSIZ || ppos + psiz > NSM2_MEMSIZ) {
    LOG("do_flushmem: psiz=%d ppos=%d\n", (int)psiz, (int)ppos);
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMEINVPTR);
    return;
  }

  /* linked list search, same check was already done in nsmlib2 */
  int nnext = (int16_t)ntohs(sys.nod[dmsg.src].noddat);
  while (nnext >= 0 && nnext < NSMSYS_MAX_DAT) {
    NSMdat& dat = sys.dat[nnext];
    dpos = (int32_t)ntohl(dat.dtpos);
    dsiz = (int32_t)ntohs(dat.dtsiz);
    if (ppos == dpos && psiz == 0) psiz = dsiz;
    if (ppos >= dpos && ppos + psiz <= dpos + dsiz) break;

    nnext = (int16_t)ntohs(dat.nnext);
    if (++n > NSMSYS_MAX_DAT) nnext = -1; /* avoid inf-loop by broken nnext */
  }
  if (nnext < 0 || nnext >= NSMSYS_MAX_DAT) {
    nsmd_tcpreply(con, dmsg, dmsg.src, NSMENOMEM);
    return;
  }

  /* send */
  ppos -= dpos;
  while (psiz > 0) {
    dsiz = nsmd_fmtcpy(udpbuf.dat, nnext, 0, ppos, psiz);
    if (dsiz < 0) {
      ASRT("flushmem: dat=%d off=%d siz=%d ret=%d", nnext, ppos, psiz, dsiz);
    }
    //DBG("udpsend nnext=%d ppos=%d dsiz=%d/%d\n", nnext, ppos, dsiz, psiz);
    nsmd_udpsend(0, NSMCMD_USRCPYMEM, nnext, ppos, &udpbuf, dsiz);
    ppos += dsiz;
    psiz -= dsiz;
  }

  nsmd_tcpreply(con, dmsg, dmsg.src, nnext);
}
// -- nsmd_command ------------------------------------------------------
// dispatch command
// ----------------------------------------------------------------------
void
nsmd_command(NSMcon& con, NSMdmsg& dmsg)
{
  NSMsys& sys = *nsmd_sysp;
  int conid = CON_ID(con);
  char pars[256];

  switch (dmsg.npar) {
    case 0: strcpy(pars, "[]"); break;
    case 1: sprintf(pars, "[%d]", dmsg.pars[0]); break;
    case 2: sprintf(pars, "[%d,%d]", dmsg.pars[0], dmsg.pars[1]); break;
    default: sprintf(pars, "[%d,%d,..(%d)]",
                       dmsg.pars[0], dmsg.pars[1], dmsg.npar);
  }

  // finding nsmd request
  if (dmsg.req >= NSMCMD_FIRST && dmsg.req <= NSMCMD_LAST) {
    NSMcmdtbl_t& cmd = nsmd_cmdtbl[dmsg.req - NSMCMD_FIRST];
    if (cmd.req != dmsg.req) ASRT("command: table broken req(%d)", dmsg.req);
    if (cmd.printable && dmsg.datap && dmsg.len < 256) {
      char buf[300];
      int i;
      for (i = 0; i < 255 && isprint(dmsg.datap[i]); i++) buf[i] = dmsg.datap[i];
      buf[i] = 0;
      if (dmsg.len != strlen(buf) + 1) {
        sprintf(buf + strlen(buf), " len=%d", dmsg.len);
      }
      LOG("recv %s(%d,%d)<=%s(%d,%d) p=%s %s",
          cmd.name, dmsg.dest, dmsg.seq,
          ADDR_STR(con.sockad), dmsg.src, conid, pars, buf);
    } else {
      int dispflag = 1;
      if ((cmd.req == NSMCMD_USRCPYMEM ||
           cmd.req == NSMCMD_SYSCPYMEM ||
           cmd.req == NSMCMD_FLUSHMEM ||
           cmd.req == NSMCMD_PING ||
           cmd.req == NSMCMD_PONG) && !DBGBIT(7)) dispflag = 0;

      if (dispflag) {
        if (dmsg.len) {
          sprintf(pars + strlen(pars), " len=%d", dmsg.len);
        }
        LOG("recv %s(%d,%d)<=%s(%d,%d) p=%s",
            cmd.name, dmsg.dest, dmsg.seq,
            ADDR_STR(con.sockad), dmsg.src, conid, pars);
      }
    }
    cmd.func(con, dmsg);
    return;
  } else {
    nsmd_sendreq(dmsg);
  }
}
// --                -- udp calls --
//                   ---------------
// -- nsmd_udpinvalid ---------------------------------------------------
// In the refused table, the latest one is always moved to the top of the
// list, and the table always keeps the top latest 16 addresses.
// ----------------------------------------------------------------------
void
nsmd_udpinvalid(int fromaddr)
{
  static struct { int ip; int n; } refused[17];
  int i;
  for (i = 0; i < 16; i++) if (refused[i].ip == fromaddr) break;
  int tmp = refused[i].n; // refused[16] is always 0.
  if (i != 0) {
    memcpy(refused + 1, refused, i * sizeof(refused[0]));
    refused[0].ip = fromaddr;
  }
  refused[0].n = tmp + 1;

  if (tmp == 0 || tmp == 9 || tmp == 99 || tmp == 999 || (tmp % 10000) == 9999) {
    WARN("udpinvalid: from %s", NODE_STR(fromaddr));
  }
}
// -- nsmd_udprecv ------------------------------------------------------
//
// - Either broadcast, or messages before connection is established.
// - NSMcon is used only to store sock and sockad, to deliver
//   the return address of an UDP message in nsmd_command.
// - NSMcon.sock points to udp.sock, so nsmd_command can tell UDP or TCP.
// - Broadcast or not is not distinguished by recvfrom, nor in NSMudpbuf.
// - UDP is only for a short packet within the MTU size.
// ----------------------------------------------------------------------
void
nsmd_udprecv()
{
  NSMsys& sys = *nsmd_sysp;
  NSMcon& udp = sys.con[NSMCON_UDP];

  NSMcon fromcon;
  memset(&fromcon, 0, sizeof(fromcon));

  // -- recvfrom
  NSMudpbuf recvbuf;
  uint32_t fromlen = sizeof(SOCKAD_IN);
  int recvlen = RECVFROM(udp.sock, recvbuf, &fromcon.sockad, &fromlen);

  if (recvlen < 0) {  // not in NSM1, probably something different is needed
    WARN("udprecv: recvfrom (%s)", strerror(errno));
    return;
  }

  // -- check the from address
  int fromaddr  = ADDR_IP(fromcon.sockad);
  int bcastaddr = ADDR_IP(udp.sockad);
  int network   = bcastaddr & sys.netmask;

  if ((fromaddr & bcastaddr) != fromaddr // from outside of broacast range
      || (fromaddr & sys.netmask) != network) { // not this network
    nsmd_udpinvalid(fromaddr);
    return;
  }
  if (fromaddr == nsmd_myip) return;

  //DBG("udprecv: recvfrom %s", ADDR_STR(fromaddr));

  if (recvlen < 4) {
    WARN("udprecv: too short received size %d < %d", recvlen, 4);
    return;
  }

  // -- it's a valid udp message
  NSMcon& udpcon = sys.con[NSMCON_UDP];
  udpcon.icnt++;
  udpcon.isiz += recvlen;

  // -- set up NSMdmsg
  NSMdmsg dmsg;
  memset(&dmsg, 0, sizeof(dmsg));
  dmsg.req = recvbuf.req + NSMCMD_UDPOFFSET;
  dmsg.seq = ntohs(recvbuf.seq);
  if (recvbuf.npar) {
    dmsg.npar = recvbuf.npar;
    for (int i = 0; i < dmsg.npar; i++) {
      dmsg.pars[i] = ntohl(*(int32_t*)&recvbuf.dat[i * sizeof(int32_t)]);
    }
  } else {
    dmsg.npar = 2; // fixed
    dmsg.pars[0] = ntohs(recvbuf.par1);
    dmsg.pars[1] = ntohs(recvbuf.par2);
  }

  int headlen = ((recvbuf.dat - (char*)&recvbuf));
  dmsg.len = recvlen - headlen - recvbuf.npar * sizeof(int32_t);
  dmsg.datap = dmsg.len ? &recvbuf.dat[recvbuf.npar * sizeof(int32_t)] : 0;
  dmsg.src = dmsg.dest = -1;
  dmsg.from = fromaddr;

  // -- call nsmd_command
  fromcon.sock = udp.sock;
  nsmd_command(fromcon, dmsg);
}
//                   ---------------
// --                -- tcp calls --
//                   ---------------
// -- nsmd_tcpaccept ----------------------------------------------------
// ----------------------------------------------------------------------
void
nsmd_tcpaccept()
{
  NSMsys& sys = *nsmd_sysp;
  NSMcon& tcp = sys.con[NSMCON_TCP];
  NSMcon& udp = sys.con[NSMCON_UDP];
  SOCKAD_IN sockad;
  uint32_t sockadlen = sizeof(SOCKAD_IN);

  // accept system call
  int sock = accept(tcp.sock, (SOCKAD*)&sockad, &sockadlen);
  if (sock < 0) {
    WARN("tcpaccept: accept (%s)", strerror(errno));
    return;
  }

  tcp.icnt++;

  // check the from address
  int fromaddr  = ADDR_IP(sockad);
  int bcastaddr = ADDR_IP(udp.sockad);

  if ((fromaddr & bcastaddr) != fromaddr) {
    LOG("REFUSE connection from (%08x)", fromaddr);
    close(sock);
    return;
  }

  // check existing connections
  int conid;
  for (conid = NSMCON_OUT; conid < sys.ncon; conid++) {
    if (ADDR_IP(sys.con[conid].sockad) == fromaddr) {
      if (ConidIsLocal(conid)) {
        continue;
      } else if (sys.con[conid].sock >= 0) {
        LOG("tcpaccept: shutdown existing connection from (%08x)", fromaddr);
        shutdown(sys.con[conid].sock, 2);
        close(sys.con[conid].sock);
        sys.con[conid].sock = -1;
        sys.sock_updated = 1;
      }
      break;
    }
  }
  const char* extrastr = "";
  if (conid < sys.ncon) {
    extrastr = " (recovered)";
  } else if (sys.ncon >= NSMSYS_MAX_CON) {
    WARN("REFUSE connection from %s (no more conn)", ADDR_STR(sockad));
    close(sock);
    return;
  } else {
    if (fromaddr == ADDR_IP(tcp.sockad)) {
      extrastr = " (local)";
    }
    recv_bsizbuf[sys.ncon - NSMCON_OUT] = 0;
    sys.ncon++;
  }
  LOG("ACCEPT connection from %s sock=%d conn=%d%s",
      ADDR_STR(sockad), sock, conid, extrastr);

  // set socket size, etc
  int TRUE = 1;
  int SIZE = nsmd_tcpsocksiz;
  if (SOCKOPT(sock, SO_REUSEADDR, TRUE) < 0)
    ERRO("tcpaccept: sockopt reuseaddr"); // unexpected
  if (SOCKOPT(sock, SO_SNDBUF, SIZE) < 0)
    ERRO("tcpaccept: sockopt sndbuf"); // unexpected
  if (SOCKOPT(sock, SO_RCVBUF, SIZE) < 0)
    ERRO("tcpaccept: sockopt rcvbuf"); // unexpected

  time_t now = time(0);
  NSMcon& con = sys.con[conid];
  con.nid = -1;
  con.sockad = sockad;
  con.sock   = sock;
  con.pid    = -1; // to be updated later for local
  con.status = 0;
  con.ready  = 0;
  con.timevent = now;
  con.timstart = 0; // to be received with NSMCMD_READY

  con.icnt = 0;
  con.ocnt = 0;
  con.isiz = 0;
  con.osiz = 0;

  sys.sock_updated  = 1;

  if (ConIsLocal(con)) {
    // if it is a local client, send my pid

    static uid_t myeuid = -1;
    if (myeuid == -1) {
      myeuid = geteuid();
      LOG("tcpaccept: euid=%d", myeuid);
    }

    int size = write(sock, (char*)&myeuid, sizeof(myeuid));

    if (size < 0) {
      WARN("tcpaccept: write (%s)", strerror(errno));
    } else if (size < sizeof(myeuid)) {
      WARN("tcpaccept: cannot write %d byte (%d byte only)",
           sizeof(myeuid), size);
    } else {
      con.ocnt++;
      con.osiz += size;
    }
  }
}
// -- nsmd_tcprecv ------------------------------------------------------
//
// - Most of the messages must be handled by a short buffer, but a long
//   buffer is at least needed to receive NSMsys memory, if I'm not the
//   first master.
//
// - One fixed long buffer and three fixed short buffers are there from
//   the beginning.  (MASTER, DEPUTY, one client would be a typical set.)
//
// - More buffers are allocated when needed
//
// - Allocated short buffer is fixed to the connection, will be reused
//   in tcprecv, and will not be freed anywhere.
//
// - Allocated long buffer is fixed to the message, and will not be reused
//   in tcprecv, and will be freed elsewhere.
// ----------------------------------------------------------------------
int xignore_recvlen0 = 0;
void
nsmd_tcprecv(NSMcon& con)
{
  NSMsys& sys = *nsmd_sysp;
  int recvlen;
  int recvsiz;
  char* recvp;
  char* datap = 0;
  int coni  = CON_ID(con) - NSMCON_OUT; // instead of conid

  if (coni < 0 || coni >= MAX_CONI)
    ASRT("tcprecv conid=%d", coni + NSMCON_OUT);

  if (! recv_shortbufp[coni]) {
    recv_shortbufp[coni] = (char*)nsmd_malloc("tcprecv", NSM_TCPTHRESHOLD);
  }

  // headlen is valid only after receiving the NSMtcphead part
  NSMtcphead& head = *(NSMtcphead*)recv_shortbufp[coni];
  int datlen = head.npar * sizeof(int32_t) + ntohs(head.len);
  int msglen = sizeof(NSMtcphead) + datlen;
  int& bsiz = recv_bsizbuf[coni];

  if (con.icnt == 0) bsiz = 0;
  con.icnt++;

  // set up recvsiz and recvp for header or main part
  if (bsiz < 0 || bsiz >= NSM_TCPBUFSIZ) { // broken con
    WARN("broken bsiz=%d for con=%d", bsiz, coni + NSMCON_OUT);
    goto disconnect_return;
  } else if (bsiz == 0) { // header
    recvsiz = sizeof(NSMtcphead);
    recvp = (char*)&head;
  } else if (bsiz < sizeof(NSMtcphead)) { // still partial header
    recvsiz = sizeof(NSMtcphead) - bsiz;
    recvp = (char*)&head + bsiz;
  } else if (msglen <= NSM_TCPTHRESHOLD) { // short buffer is fine
    recvsiz = msglen - bsiz;
    recvp   = recv_shortbufp[coni] + bsiz;
    datap   = recv_shortbufp[coni] + sizeof(NSMtcphead);
  } else { // need a long buffer
    if (bsiz == sizeof(NSMtcphead)) {
      /* datap may not be freed after nsmd_command, and new message
         may be received before tcpwriteq is done. */
      if (! nsmd_longused) {
        nsmd_longused = 1;
        datap = nsmd_longbuf;
      } else {
        datap = (char*)nsmd_malloc("tcprecv", datlen);
      }
      recv_longbufp[coni] = datap;
    } else {
      datap = recv_longbufp[coni];
    }
    recvsiz = msglen - bsiz;
    recvp   = datap + (bsiz - sizeof(NSMtcphead));
  }
  if (recvsiz <= 0) ASRT("tcprecv recvsiz(%d) <= 0", recvsiz);

  //DBG("tcprecv before read");
  while (1) {
    if ((recvlen = read(con.sock, recvp, recvsiz)) >= 0) break;
    if (errno == EINTR || errno == EAGAIN) continue;
    // connection has an error
    LOG("tcprecv read: %s conid=%d sock=%d ret=%d recvp=%x",
        strerror(errno), CON_ID(con), con.sock, recvlen, recvp);
    goto disconnect_return;
  }
  //DBG("tcprecv recvlen=%d/%d recvp=%x", recvlen, recvsiz, recvp);

  // connection is disconnected
  if (recvlen == 0) {
    if (recvp == (char*)&head) {
      LOG("tcprecv connection closed recvsiz=%d", recvlen);
    } else {
      LOG("tcprecv connection closed while reading %d bytes", recvsiz);
    }
    if (xignore_recvlen0) return;
    goto disconnect_return;
  }

  // just print log and return if not fully received
  if (recvlen < recvsiz) {
    bsiz += recvlen;
    DBG("tcprecv(%d,%d) recvlen=%d/%d recvp=%x",
        con.sock, CON_ID(con), recvlen, recvsiz, recvp);
    return;
  }
  if (bsiz + recvlen == sizeof(NSMtcphead) &&
      (head.len != 0 || head.npar != 0)) { // no ntohs to check non-zero
    bsiz += recvlen;
    // just header is received and more is coming, so no need to print log
    // DBG("tcprecv recvlen=%d/%d recvp=%x", recvlen, recvsiz, recvp);
    return;
  }

  {
    unsigned char* h = (unsigned char*)&head;
    int* p = (int*)((char*)&head + sizeof(NSMtcphead));
    int npar = head.npar;
    int req  = ntohs(head.req);
    int len  = ntohs(head.len);
    int src  = ntohs(head.src);
    int dest = ntohs(head.dest);
    int from = ntohl(head.from);
    int err  = 0;
    int reqid = req - NSMREQ_FIRST;

    /* 20170927 commented out
    if (req != NSMCMD_SYSCPYMEM) {
      DBG("tcprecv(%d,%d) req=%x %d=>%d len=%d npar=%d p=[%d,%d] coni=%d",
          con.sock, CON_ID(con),
          req, src, dest, len, npar, ntohl(p[0]), ntohl(p[1]), coni);
    }
    */

    if (src  != 65535 && src  != 0 && src  >= NSMSYS_MAX_NOD) err |= 1;
    if (dest != 65535 && dest != 0 && dest >= NSMSYS_MAX_NOD) err |= 2;
    if (req < NSMREQ_FIRST || req > NSMCMD_LAST) err |= 4;
    if (reqid >= NSMSYS_MAX_REQ && req < NSMCMD_FIRST) err |= 8;
    if (! err && nsmd_sysp) {
      if (src  != 65535 && src  != 0 && sys.nod[src].name[0]  == 0) err |= 16;
      if (dest != 65535 && dest != 0 && sys.nod[dest].name[0] == 0) err |= 32;
      if (reqid < NSMSYS_MAX_REQ    && sys.req[reqid].name[0] == 0) err |= 64;
    }
    if (err) {

      /* 20170927 moved to here */
      LOG("tcprecv(%d,%d) req=%x %d=>%d len=%d npar=%d p=[%d,%d] coni=%d",
          con.sock, CON_ID(con),
          req, src, dest, len, npar, ntohl(p[0]), ntohl(p[1]), coni);

      for (int i = 0; i < 32; i += 8) {
        LOG("tcphead %02x %02x %02x %02x - %02x %02x %02x %02x",
            h[i + 0], h[i + i], h[i + 2], h[i + 3], h[i + 4], h[i + 5], h[i + 6], h[i + 7]);
      }
      ASRT("bad tcphead errcode=%x", err);
    }
  }

  con.isiz += bsiz;
  con.timevent = time(0);
  bsiz = 0;

  NSMdmsg dmsg;
  dmsg.req  = ntohs(head.req);
  dmsg.seq  = ntohs(head.seq);
  dmsg.src  = ntohs(head.src);
  dmsg.dest = ntohs(head.dest);
  dmsg.len  = ntohs(head.len);
  dmsg.from = ADDR_IP(con.sockad);
  dmsg.opt  = head.opt;
  dmsg.npar = head.npar;
  for (int i = 0; i < dmsg.npar; i++) {
    int32_t par = *(int32_t*)(datap + i * sizeof(int32_t));
    dmsg.pars[i]   = ntohl(par);
  }
  dmsg.datap = dmsg.len ? datap + dmsg.npar * sizeof(int32_t) : 0;

  nsmd_command(con, dmsg);

  /*
    If datap is still in use in tcpwriteq, tcpsend clears
    dmsg.datap and it should not be freed here.
  */
  if (dmsg.datap) {
    if (datap == nsmd_longbuf || (datap && msglen > NSM_TCPTHRESHOLD)) {
      nsmd_free("tcprecv", datap);
    }
  }

  return;

disconnect_return:

  // on error, data pointer has to be cleared
  if (datap == nsmd_longbuf || (datap && msglen > NSM_TCPTHRESHOLD)) {
    nsmd_free("tcprecv", datap);
  }

#if READYtoIMPLEMENT
  if (! ItsLocal(conid) && con.status == NSMDC_NA) {
    // can't simply destroy as it may be a master or deputy
    nsmd_reconnect(conid);
    return;
  }
#endif

  int conid = coni + NSMCON_OUT;
  LOG("DISCONNECT %s (conn=%d sock=%d nid=%d pid=%d)",
      ConidIsLocal(conid) ? NODE_STR(con.nid) : ADDR_STR(con.sockad),
      conid, con.sock, con.nid, con.pid);

#if READYtoIMPLEMENT
  if (conid == sys.master && ! sys.ready) {
    LOG("master disappeared during initialization");
    nsmd_init_count = NSMD_INITCOUNT_SECOND;
    nsmd_schedule(NSMCON_NON, NSMSCH_INITBCAST, 0, 0, 0);
  }
#endif

  if ((IamMaster() && ! ConIsLocal(con)) ||
      (IamDeputy() && ConIsMaster(con))) {
    nsmd_destroyconn(con, 1, "tcprecv");
  } else {
    nsmd_destroyconn(con, 0, "tcprecv");
  }
}
//                   ---------------
// --                -- main loop --
//                   ---------------

// -- nsmd_select -------------------------------------------------------
// ----------------------------------------------------------------------
int
nsmd_select(fd_set& fdset, int wait_in10ms)
{
  timeval tv;
  tv.tv_sec  = wait_in10ms / 100; // no wait if zero
  tv.tv_usec = wait_in10ms * 10 * 1000;

  NSMsys& sys = *nsmd_sysp;
  NSMcon& conlast = sys.con[sys.ncon];
  static fd_set fdset_saved;
  static int highest = -1;

  if (highest < 0 || sys.sock_updated) {
    highest = -1;
    FD_ZERO(&fdset);
    for (NSMcon* conp = sys.con; conp < &conlast; conp++) {
      if (conp->sock > 0) {
        FD_SET(conp->sock, &fdset);
        if (conp->sock > highest) highest = conp->sock;
      }
    }
    fdset_saved = fdset;
    sys.sock_updated = 0;
  } else {
    fdset = fdset_saved;
  }

  int nfd = -1;
  do {
    if ((nfd = select(highest + 1, &fdset, 0, 0, &tv)) < 0) {
      if (errno != EINTR) ERRO("select");
      LOG("select: interrupt!");
    }
  } while (nfd < 0);

  return nfd;
}
// -- nsmd_loop ---------------------------------------------------------
//    mainloop
// ----------------------------------------------------------------------
void
nsmd_loop()
{
  int busy = 0;
  uint64_t tinfo_in10ms = time10ms() + 30 * 100; /* 30s after the start */
  fd_set fdset_skip;
  FD_ZERO(&fdset_skip);

  while (1) {

    fd_set fdset;
    NSMsys& sys = *nsmd_sysp;

    if (! busy) {
      nsmd_shmtouch();
      if (nsmd_logfp && nsmd_logfp != stdout && nsmd_logfp != stderr) {
        fflush(nsmd_logfp);
      }
    }

    busy = 0;

    // -- check and process incoming messages
    int nfd = nsmd_select(fdset, 1); // 10msec
    if (nfd > 0) {
      NSMcon& conlast = sys.con[sys.ncon];
      int action = 0;
      for (int conid = 0; nfd > 0 && conid < sys.ncon; conid++) {
        int sock = sys.con[conid].sock;
        if (sock < 0 || ! FD_ISSET(sock, &fdset)) continue;
        if (FD_ISSET(sock, &fdset_skip)) continue;
        nfd--;
        if (conid == NSMCON_UDP) {
          nsmd_udprecv();
          FD_SET(sock, &fdset_skip);
          action++;
        } else if (conid == NSMCON_TCP) {
          nsmd_tcpaccept();
          FD_SET(sock, &fdset_skip);
          action++;
        } else {
          nsmd_tcprecv(sys.con[conid]);
          FD_SET(sock, &fdset_skip);
          action++;
        }
      }
      if (! action) FD_ZERO(&fdset_skip);
      busy = 1;
    }

    // -- schedule list
    uint64_t now_in10ms = time10ms();
    for (int i = 0; i < sys.nsch; i++) {
      NSMsch& sch = sys.sch[i];
      if (sch.when > 0 && now_in10ms >= sch.when) {
        NSMsch schsav = sch;
        int next = (*nsmd_schlist[sch.funcid].func)(sch.conid, sch.opt);
        for (; i < sys.nsch - 1; i++) {
          sys.sch[i] = sys.sch[i + 1];
        }
        if (next < 0) {
          sys.nsch--;
          memset(&sys.sch[sys.nsch], 0, sizeof(NSMsch));
        } else {
          sys.sch[sys.nsch - 1] = schsav;
          sys.sch[sys.nsch - 1].when = time10ms() + next;
        }
        busy = 1;
        break;
      }
    }

    // -- tcpwriteq
    while (1) {
      if (nsmd_tcpwriteq() == 0) break;
    }

    // -- shared memory update
    nsmd_shmcast();

    // -- nsminfo2
    if (tinfo_in10ms && now_in10ms > tinfo_in10ms) {
      struct tm tmbuf;
      time_t t = now_in10ms / 100 + 3600;
      struct tm tbuf = *localtime(&t);
      tbuf.tm_min = 0;
      tbuf.tm_sec = 0;
      LOG("nsminfo");
      nsminfo2();
      tinfo_in10ms = (uint64_t)mktime(&tbuf) * 100;
      LOG("tinfo: now %lld.%02d next %lld.%02d",
          now_in10ms / 100, (int)now_in10ms % 100,
          tinfo_in10ms / 100, (int)tinfo_in10ms % 100);
    }
  }
}
//                   ----------
// --                -- main --
//                   ----------

// -- nsmd_main ---------------------------------------------------------
//    call init and mainloop
// ----------------------------------------------------------------------
void
nsmd_main(int dryrun = 0)
{
  // -- use the start up time for random seed
  srand(time(0) | nsmd_myip);

  // show protocol version
  int dver = NSM_DAEMON_VERSION;
  int pver = NSM_PROTOCOL_VERSION;

  /*
  LOG("%s version %d.%d.%02d protocol %d.%d.%02d",
      "nsmd - network shared memory daemon",
      dver/1000, (dver/100)%10, dver%100, pver/1000, (pver/100)%10, pver%100);
  */

  nsmd_init(dryrun);

  if (! dryrun) {
    LOG("nsmd started, pid=%d, priority=%d.", getpid(), nsmd_priority);
    nsmd_loop();
  }
}
// -- main --------------------------------------------------------------
//    option analysis and process fork
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  char* av[MAX_ARGS];    // ac/av to fork and exec the same process
  int  ac = 0;           // just to leave useful info in ps output
  char av0[256];         // altered argv[0] to show up ps
  char* argv0 = argv[0]; // original argv[0] to exec
  int nofork = 0;
  int pid    = 0;
  char* logdir = 0;
  char* host = 0;
  int port   = -1;
  int shmkey = -1;
  int debug  = -1;
  int prio   = -1;
  int tcpbuf = -1;
  const char* nsmusr = 0;
  const char* nsmgrp = 0;

  av[ac++] = av0; // av0 will be filled later

  // option loop
  while (argc > 1 && argv[1][0] == '-') {
    char opt = argv[1][1];
    char* ap = &argv[1][2];
    if (strchr("pshlgu", argv[1][1]) && ! *ap) {
      av[ac++] = argv[1];
      argc--, argv++;
      ap = argv[1];
    }

    switch (opt) {
      case 'b': nofork = 0; nsmd_logfp = stdout; break; // default
      case 'f': nofork = 1; nsmd_logfp = 0;      break;
      case 'o': nofork = 1; nsmd_logfp = stdout; break;
      case 'd': debug  = nsmd_atoi(ap, 255); break;
      case 'm': prio   = nsmd_atoi(ap, 10); break;
      case 'p': port   = nsmd_atoi(ap, -1); break;
      case 's': shmkey = nsmd_atoi(ap, -1); break;
      case 't': tcpbuf = nsmd_atoi(ap, 0); break;
      case 'u': if (ap && *ap) nsmusr = ap; break;
      case 'g': if (ap && *ap) nsmgrp = ap; break;
      case 'h': if (ap && *ap) host   = ap; break;
      case 'l': if (ap && *ap) logdir = ap; break;
      default:
        printf("usage: nsmd2 [options]\n");
        printf(" -f         run as foreground, message into a log file.\n");
        printf(" -o         run as foreground, message into stdout.\n");
        printf(" -b         run as a background process (default).\n");
        printf(" -p <port>  set port number.\n");
        printf(" -h <host>  set host name.\n");
        printf(" -s <key>   set shmkey number.\n");
        printf(" -l <log>   set log file directory/prefix.\n");
        printf(" -t <size>  set TCP buffer size.\n");
        printf(" -u <user>  shm uid when run as root (default=nsm).\n");
        printf(" -g <group> shm gid when run as root (default=nsm).\n");
        printf(" -d<num>    set debug level (0-255, default=0).\n");
        printf(" -d         set debug level to 255.\n");
        printf(" -m<pri>    set mastership priority (0-100, default=1).\n");
        printf(" -m         set mastership priority to=10.\n");
        exit(1);
    }
    av[ac++] = argv[1];
    argc--, argv++;
    if (ac >= MAX_ARGS - (nofork ? 1 : 2)) {
      printf("nsmd2: too many args (up to 30)\n");
      exit(1);
    }
  }

  // environmental variables
  if (! host)     host   = getenv(NSMENV_HOST);
  if (! logdir)   logdir = getenv(NSMDENV_LOGDIR);
  if (debug < 0)  debug  = nsmd_atoi(getenv(NSMDENV_DEBUG));
  if (prio < 0)   prio   = nsmd_atoi(getenv(NSMDENV_PRIORITY), 1);
  if (port < 0)   port   = nsmd_atoi(getenv(NSMENV_PORT));
  if (shmkey < 0) shmkey = nsmd_atoi(getenv(NSMENV_SHMKEY), -1);
  if (tcpbuf < 0) shmkey = nsmd_atoi(getenv(NSMDENV_TCPBUF), -1);
  if (! nsmusr)   nsmusr = getenv(NSMDENV_USER);
  if (! nsmgrp)   nsmgrp = getenv(NSMDENV_GROUP);

  // network related global variables
  memset(nsmd_host, 0, sizeof(nsmd_host));
  if (host) strncpy(nsmd_host, host, sizeof(nsmd_host));
  if (! host) gethostname(nsmd_host, sizeof(nsmd_host));
  if (! nsmd_host[0]) ERRO("can't gethostname");

  hostent* hp = gethostbyname(nsmd_host);
  if (! hp) ERRO("can't gethostbyname(%s)", nsmd_host);
  memset(&nsmd_sockad, 0, sizeof(nsmd_sockad));
  memcpy(&nsmd_sockad.sin_addr, hp->h_addr, hp->h_length);
  nsmd_myip = ADDR_IP(nsmd_sockad);

  // final set of global variables
  if (logdir)      nsmd_logdir = logdir;
  if (debug >= 0)  nsmd_debug  = debug;
  if (prio >= 0)   nsmd_priority = prio;
  if (port > 0)    nsmd_port   = port;
  if (shmkey >= 0) nsmd_shmkey = shmkey;
  if (nsmd_shmkey < 0) nsmd_shmkey = nsmd_port;
  if (tcpbuf >= 65536) nsmd_tcpsocksiz = tcpbuf;
  if (nsmusr)      nsmd_usrnam = nsmusr;
  if (nsmgrp)      nsmd_grpnam = nsmgrp;

  // start process
  if (nofork) {
    nsmd_main(0);
    return 0; // will not reach here

  } else {
    // dry run to check if it can initialize successfully
    nsmd_main(1);

    if ((pid = fork()) < 0) {
      printf("nsmd2: cannnot fork\n");
      exit(1);
    }
  }

  // exec myself again if forked
  if (pid) {
    /* there may be a better way to wait until the child process
       is properly started than just sleep 1 second. */
    sleep10ms(100, 1);
    LOG("nsmd started in background, pid=%d, priority=%d.",
        pid, nsmd_priority);

    return 0; // parent process terminates now
  } else {
    // final touch to the arg list and exec
    sprintf(av0, "nsmd2:%d", nsmd_port);
    if (nsmd_shmkey != nsmd_port) {
      sprintf(av0 + strlen(av0), ":%d", nsmd_shmkey);
    }
    av[ac++] = (char*)"-f";
    av[ac] = 0;
    execvp(argv0, av);
  }

  return 0; // will not reach here
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
