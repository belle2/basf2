// ----------------------------------------------------------------------
// -- nsminfo2.cc
//
// 20131218  xforce option [from nsmd 1913]
// 20131229  shmget permission changed
// 20140104  disid / conid
// 20140105  integrating into nsmd2
// 20140113  fix: mem shared memory mode 0664 -> 0644
// 20140115  nsmd/protocol versions
// 20140117  color version
// 20140117  xcolor version
// 20140124  anonymous, one less column
// ----------------------------------------------------------------------

// -- include files -----------------------------------------------------
// ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "nsm2.h"
#include "nsmsys2.h"

static int xcolor = 0;
#define BLACK()   if (xcolor) printf("\033[m")
#define BOLD()    if (xcolor) printf("\033[0;1m")
#define RED()     if (xcolor) printf("\033[40;31;1m")
#define CYAN()    if (xcolor) printf("\033[40;36;1m")
#define GREEN()   if (xcolor) printf("\033[40;32;1m")
#define YELLOW()  if (xcolor) printf("\033[40;33;1m")
#define BLUE()    if (xcolor) printf("\033[47;34;1m")
#define MAGENTA() if (xcolor) printf("\033[35;1m")


// -- standalone or called from nsmd2 -----------------------------------
// ----------------------------------------------------------------------
#ifdef NSMD2INFO
#define P nsmd_print0
#define PP nsmd_print1
extern void nsmd_print0(const char* fmt, ...);
extern void nsmd_print1(const char* fmt, ...);
extern NSMsys* nsmd_sysp;
extern NSMmem* nsmd_memp;
extern int nsmd_shmkey;

static int xforce = 1;
static int xverbose = 0;
static int showalist = 1;
static int showdisid = 1;
static int showconid = 1;

#else /* ! NSMD2INFO */
#define P printf
#define PP printf

int nsmd_port = NSM2_PORT;
int nsmd_shmkey = -1; /* == nsmd_port if -1 */

NSMsys* nsmd_sysp = 0;
NSMmem* nsmd_memp = 0;

static int xforce = 0;
static int xverbose = 0;
static int showalist = 0;
static int showdisid = 0;
static int showconid = 0;

#endif /* NSMD2INFO */

// -- macros ------------------------------------------------------------
// ----------------------------------------------------------------------

#define S sprintf /* to avoid printf search */
#define SYSPOS(ptr) ((char *)(ptr) - (char *)nsmd_sysp)
#define SYSPTR(pos) (((char *)nsmd_sysp)+pos)
#define MEMPOS(base,ptr) ((char *)(ptr) - (char *)(base))
#define MEMPTR(base,off) ((char *)(base) + off)

#define ADDR_IP(a)    ((uint32)((a)->sin_addr.s_addr))

#define IamMaster()    (nsmd_sysp->master == NSMCON_TCP)
#define IamDeputy()    (nsmd_sysp->deputy == NSMCON_TCP)
#define ItsLocal(conn) (AddrConn(conn) == AddrSelf())

#define AddrSelf()     AddrConn(NSMCON_TCP)
#define AddrMaster()   AddrConn(nsmd_sysp->master)
#define AddrDeputy()   AddrConn(nsmd_sysp->deputy)
#define AddrConn(conn) ADDR_IP(&nsmd_sysp->con[conn].sockad)

// -- forward declarations ----------------------------------------------
// ----------------------------------------------------------------------
extern "C" int nsmlib_hash(NSMsys* sysp, int32* hashtable, int hashmax,
                           const char* key, int create);

// -- hoststr -----------------------------------------------------------
static const char*
nsminfo_hoststr(int ip, int iponly = 0) // ip: network byte order
{
  static char host[16][256];
  static int ihost = 0;
  int iph = ntohl(ip);

  memset(host[ihost], 0, sizeof(host[ihost]));
  char* hostp = host[ihost];
  ihost = (ihost + 1) % 16;

  iponly = 1;

  if (iponly) {
    S(hostp, "%d.%d.%d.%d",
      (iph >> 24) & 255, (iph >> 16) & 255, (iph >> 8) & 255, (iph >> 0) & 255);
  }

  return hostp;
}
// -- timestr -----------------------------------------------------------
static const char*
nsminfo_timestr(time_t t, int typ = 0)
{
  static char timestr[16][256];
  static int itime = 0;

  memset(timestr[itime], 0, sizeof(timestr[itime]));
  char* timep = timestr[itime];
  itime = (itime + 1) % 16;

  if (typ == 0) {
    if (t == 0) return "unknown";

    time_t now = time(0);
    int dt = now - t;
    if (dt < 0) {
      return "negative-time";
    } else if (dt < 60) {
      S(timep, "%ds", dt);
    } else if (dt < 60 * 60) {
      S(timep, "%dm%02ds", dt / 60, dt % 60);
    } else if (dt < 60 * 60 * 24) {
      S(timep, "%dh%02dm", (dt / 60) / 60, (dt / 60) % 60);
    } else {
      S(timep, "%dd%02dh", (dt / 3600) / 24, (dt / 3600) % 24);
    }
  } else {
    if (t == 0) return "unknown-time";

    tm tmbuf;
    localtime_r(&t, &tmbuf);
    S(timep, "%04d.%02d.%02d-%02d:%02d:%02d",
      tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday,
      tmbuf.tm_hour, tmbuf.tm_min, tmbuf.tm_sec);
  }

  return timep;
}
// -- head --------------------------------------------------------------
static int
nsminfo_head()
{
  NSMsys& sys = *nsmd_sysp;
  NSMmem& mem = *nsmd_memp;
  pid_t  pid    = sys.pid;
  time_t tstart = sys.timstart;
  time_t tevent = sys.timevent;
  int16_t dver = sys.version;
  int16_t pver = sys.protocol;
  int badmem = 0;
  NSMcon& master = sys.con[sys.master];
  NSMcon& udpcon = sys.con[NSMCON_UDP];
  NSMcon& tcpcon = sys.con[NSMCON_TCP];

  if (tstart == (time_t) - 1) {
    // so far not successful to gracefully reattach to newly
    // restarted nsmd2
    time_t now = time(0);
    if (now - tevent >= 0 && now - tevent < 3600) {
      P("NSMD has terminated at %s (%s ago).\n",
        nsminfo_timestr(tevent, 1),
        nsminfo_timestr(tevent));
    } else {
      P("NSMD has terminated.\n");
    }
    exit(1);
  } else if (sys.master == NSMCON_NON) {
    P("NSM network is not started yet\n");
  } else {
    P("NSM network v%d.%d.%02d started at %s (%s ago) gen=%d\n",
      pver / 1000, (pver % 1000) / 100, pver % 100,
      nsminfo_timestr(tcpcon.timstart, 1),
      nsminfo_timestr(tcpcon.timstart),
      sys.generation);
  }
  P("NSMD daemon v%d.%d.%02d started at %s (%s ago) priority=%d\n",
    dver / 1000, (dver % 1000) / 100, dver % 100,
    nsminfo_timestr(udpcon.timstart, 1),
    nsminfo_timestr(udpcon.timstart),
    sys.priority);

  if (sys.master == NSMCON_NON) {
    RED();
  } else if (sys.master == NSMCON_TCP) {
    CYAN();
  } else if (sys.deputy == NSMCON_TCP) {
    GREEN();
  } else {
    YELLOW();
  }

  P("MASTER(%d) %s %s, DEPUTY(%d) %s %s, I'm %s nsmd\n",
    sys.master,
    sys.master == NSMCON_NON ? "is" : "at",
    sys.master == NSMCON_NON ? "missing" : nsminfo_hoststr(AddrMaster()),
    sys.deputy,
    sys.deputy == NSMCON_NON ? "is" : "at",
    sys.deputy == NSMCON_NON ? "missing" : nsminfo_hoststr(AddrDeputy()),
    sys.master == NSMCON_TCP ? "the MASTER" :
    sys.deputy == NSMCON_TCP ? "the DEPUTY" : "a MEMBER");

  RED();
  if (nsmd_memp) {
    if (sys.ipaddr != mem.ipaddr) {
      P("inconsistent ipaddr: %s(sys) vs %s(mem)\n",
        nsminfo_hoststr(sys.ipaddr, 1), nsminfo_hoststr(mem.ipaddr, 1));
      badmem++;
    }
    if (pid != mem.pid) {
      P("inconsistent process-id: %d vs %d\n", pid, mem.pid);
      badmem++;
    }
    if (tstart != mem.timstart) {
      P("inconsistent start time: %d vs %d\n",
        (int)tstart, (int)mem.timstart);
      badmem++;
    }
    if (abs(tevent - mem.timevent) > 1) {
      P("inconsistent time: %d vs %d\n",
        (int)tevent, (int)mem.timevent);
      badmem++;
    }
  }
  BLACK();
  if (badmem && ! xforce) {
    exit(1);
  }

  P("NSMD shmkey = %d, ip = %s, pid = %d\n",
    nsmd_shmkey, nsminfo_hoststr(sys.ipaddr, 1),
    nsmd_sysp->pid);
  P("     created %s ago, last updated %s ago%s\n",
    nsminfo_timestr(sys.timstart),
    nsminfo_timestr(sys.timevent),
    (time(0) - sys.timevent <= 2) ? " (up-to-date)" : "");

  return 0;
}
// -- conn --------------------------------------------------------------
static void
nsminfo_conn(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;
  int ncon = all ? NSMSYS_MAX_CON : sys.ncon;

  for (int icon = NSMCON_OUT; icon < ncon; icon++) {
    NSMcon& con = sys.con[icon];
    char name[32];

    if (icon == sys.master) {
      CYAN();
    } else if (icon == sys.deputy) {
      GREEN();
    } else if (ItsLocal(icon)) {
      BOLD();
    } else {
      YELLOW();
    }

    if (icon == NSMCON_TCP) {
      strcpy(name, "name=[TCP]");
    } else if (icon == NSMCON_UDP) {
      strcpy(name, "name=[BCAST]");
    } else if (ItsLocal(icon)) {
      //S(name, "name=%s", sys->nod[sys->con[i].nid].name);
      S(name, "nodeid=%d", con.nid);
    } else {
      S(name, "host=%s", nsminfo_hoststr(AddrConn(icon)));
    }
    char nidbuf[32];
    if (con.nid == NSMSYS_MAX_NOD) {
      strcpy(nidbuf, "---");
    } else {
      sprintf(nidbuf, "%-3.1d", con.nid);
    }

    P("CON %-3.1d nid=%s pid=%-5d sock=%-2d rtim=%-6.6s ev=%-6.6s %s%s\n",
      icon,
      nidbuf, con.pid, con.sock,
      nsminfo_timestr(con.timstart),
      nsminfo_timestr(con.timevent),
      name,
      (icon == sys.master) ? " M" : ((icon == sys.deputy) ? " D" : ""));
  }
  BLACK();
}
// -- node --------------------------------------------------------------
static void
nsminfo_node(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int inod = 0; inod < NSMSYS_MAX_NOD; inod++) {
    NSMnod& nod = sys.nod[inod];
    if (! nod.name[0]) continue;

    if (nod.ipaddr) MAGENTA();

    P("NOD %-3.1d %-31.31s pid/uid=%-5.1d/%-5.1d @%s %d\n",
      inod, nod.name, (int32)ntohl(nod.nodpid), ntohl(nod.noduid),
      nsminfo_hoststr(nod.ipaddr), (int16_t)ntohs(nod.noddat));

    BLACK();
  }
}
// -- req ---------------------------------------------------------------
static void
nsminfo_req(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int ireq = 0; ireq < NSMSYS_MAX_REQ; ireq++) {
    NSMreq& req = sys.req[ireq];
    if (! req.code || ! req.name[0]) continue;

    int hash = nsmlib_hash(&sys, sys.reqhash, NSMSYS_MAX_HASH, req.name, 0);
    int reqpos = ntohl(sys.reqhash[hash]);
    NSMreq* reqp = (NSMreq*)SYSPTR(reqpos);

    P("REQ %-3.1d %-31.31s code=%04x hash=%d(%d,%04x)\n",
      ireq, req.name, ntohs(req.code), hash, reqpos,
      hash >= 0 && reqpos > 0 && reqpos < sizeof(sys)
      ? ntohs(reqp->code) : -1);
  }
}
// -- dat ---------------------------------------------------------------
static void
nsminfo_dat(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int idat = 0; idat < NSMSYS_MAX_DAT; idat++) {
    NSMdat& dat = sys.dat[idat];
    if (dat.dtsiz == 0 && xverbose == 0) continue;
    if (dat.owner == (int16_t) - 1) {
      RED();
    } else {
      MAGENTA();
    }
    if (xverbose) {
      P("DAT %-3.1d %-31.31s pos=%d sz=%d nod=%d rev=%d ref=%d fmt=%s",
        idat, dat.dtnam, ntohl(dat.dtpos), ntohs(dat.dtsiz),
        (int16_t)ntohs(dat.owner), ntohs(dat.dtrev), ntohs(dat.dtref),
        dat.dtfmt);
    } else {
      P("DAT %-3.1d %-31.31s sz=%d nod=%d rev=%d ref=%d fmt=%s",
        idat, dat.dtnam, ntohs(dat.dtsiz),
        (int16_t)ntohs(dat.owner), ntohs(dat.dtrev), ntohs(dat.dtref),
        dat.dtfmt);
    }
    if (dat.dtfmt[strlen(dat.dtfmt) + 1]) {
      P(" %s", dat.dtfmt + strlen(dat.dtfmt) + 1);
    }
    P("\n");
    BLACK();
  }
}
// -- ref ---------------------------------------------------------------
static void
nsminfo_ref(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int iref = 0; iref < NSMSYS_MAX_DAT; iref++) {
    NSMref& ref = sys.ref[iref];
    if (ref.refnod == -1) continue;
    P("REF %-3.1d: %-3.1d => %-3.1d\n",
      iref, ntohs(ref.refnod), ntohs(ref.refdat));
  }
}
// -- disid -------------------------------------------------------------
static void
nsminfo_disid(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  P("nsnd = %d\n", sys.nsnd);

  for (int i = 0; i < sys.nsnd; i++) {
    P("SND%d disid=%d disnod=%d\n",
      i, sys.snd[i].disid, sys.snd[i].disnod);
  }
}
// -- conid -------------------------------------------------------------
static void
nsminfo_conid(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;
  int nnod = (int16_t)ntohs(sys.nnod);

  for (int i = 0; i < nnod; i++) {
    if ((i % 10) == 0) P("CONID");
    PP(" %d=>%d", i, sys.conid[i]);
    if ((i % 10) == 9 || i == nnod - 1) PP("\n");
  }
}
// -- alist -------------------------------------------------------------
static void
nsminfo_alist(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;
  int anext = (int16_t)ntohs(sys.afirst);
  int amax = 1024;

  P("alist = %d", anext);
  while (amax-- > 0 && anext != -1) {
    NSMdat& dat = sys.dat[anext];
    anext = (int16_t)ntohs(dat.anext);
    PP(" %d", anext);
  }
  PP("\n");
}
// -- nsminfo2 ----------------------------------------------------------
//
// ----------------------------------------------------------------------
void
nsminfo2()
{
  if (! nsmd_sysp || ! nsmd_memp) {
    P("nsminfo2: no shared memory.\n");
    return;
  }

  if (nsminfo_head() < 0) return;
  nsminfo_conn();
  nsminfo_node();
  nsminfo_req();
  nsminfo_dat();
  nsminfo_ref();

  if (showalist) nsminfo_alist();
  if (showdisid) nsminfo_disid();
  if (showconid) nsminfo_conid();
}
// -- nsmd_atoi ---------------------------------------------------------
//    a bit more intelligent than atoi
// ----------------------------------------------------------------------
#ifndef NSMD2INFO
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
#endif
// -- init --------------------------------------------------------------
#ifndef NSMD2INFO
void
nsminfo2_init(int nsmd_shmkey)
{
  int id;
  if ((id = shmget(nsmd_shmkey, sizeof(*nsmd_sysp), 0444)) < 0) {
    if (errno = ENOENT) {
      printf("nsmd2 is not running at port %d\n", nsmd_shmkey);
    } else {
      printf("shmkey (sys) = %d\n", nsmd_shmkey);
      perror("shmget (sys):");
    }
    exit(1);
  }
  nsmd_sysp = (NSMsys*)shmat(id, 0, SHM_RDONLY);
  if (nsmd_sysp == (NSMsys*) - 1) {
    printf("shmkey (sys) = %d\n", nsmd_shmkey);
    perror("shmat (sys):");
    exit(1);
  }
  if ((id = shmget(nsmd_shmkey + 1, sizeof(*nsmd_memp), 0444)) < 0) {
    if (errno = ENOENT) {
      printf("Cannot open MEM shared memory with key=%d. %s\n",
             nsmd_shmkey + 1, "Something is inconsistent");
    } else {
      printf("shmkey (mem) = %d\n", nsmd_shmkey + 1);
      perror("shmget (mem):");
    }
    return;
  }
  nsmd_memp = (NSMmem*)shmat(id, 0, SHM_RDONLY);
  if (nsmd_memp == (NSMmem*) - 1) {
    printf("shmkey (mem) = %d\n", nsmd_shmkey + 1);
    perror("shmat (mem):");
    nsmd_memp = 0;
  }
}
#endif /* NSMD2INFO */
// -- handler -----------------------------------------------------------
//    to restore escape sequence if colored
// ----------------------------------------------------------------------
/* ---------------------------------------------------------------------- *\
   handler
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
handler(int sig)
{
  if (xcolor) {
    printf("\033[m");
  }
  exit(0);
}
#endif
// -- main --------------------------------------------------------------
//
// ----------------------------------------------------------------------
#ifndef NSMD2INFO
int
main(int argc, char** argv)
{
  int port = -1;
  int shmkey = -1;
  int interval = -1;

  // option loop
  while (argc > 1 && argv[1][0] == '-') {
    char opt = argv[1][1];
    char* ap = &argv[1][2];

    switch (opt) {
      case 'f':
        xforce = 1;
        argc--, argv++;
        continue;
      case 'v':
        xverbose = 1;
        argc--, argv++;
        continue;
    }

    if (strchr("psi", argv[1][1]) && ! *ap && argc > 2 && argv[2][0] != '-') {
      argc--, argv++;
      ap = argv[1];
    }

    switch (opt) {
      case 'i': interval  = nsmd_atoi(ap,  2); xcolor = 1; break;
      case 'c': xcolor    = 1; break;
      case 'C': xcolor    = 0; break;
      case 'p': port      = nsmd_atoi(ap, -1); break;
      case 's': shmkey    = nsmd_atoi(ap, -1); break;
      case 'A': showalist = 1; break;
      case 'D': showdisid = 1; break;
      case 'X': showconid = 1; break;
      default:
        printf("usage: nsminfo2 [options]\n");
        printf(" -p <port>   set port number.\n");
        printf(" -s <shmkey> set shmkey number.\n");
        printf(" -i[<n>] repeat every n(default=2) seconds\n");
        exit(1);
    }
    argc--, argv++;
  }

  if (port < 0)   port   = nsmd_atoi(getenv(NSMENV_PORT));
  if (shmkey < 0) shmkey = nsmd_atoi(getenv(NSMENV_SHMKEY), -1);

  if (port > 0)    nsmd_port   = port;
  if (shmkey >= 0) nsmd_shmkey = shmkey;
  if (nsmd_shmkey < 0) nsmd_shmkey = nsmd_port;


  nsminfo2_init(nsmd_shmkey);

  if (interval > 0) {
    signal(SIGINT,  handler);
    signal(SIGTERM, handler);
    signal(SIGSEGV, handler);
    signal(SIGHUP,  handler);
    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);

    while (1) {
      printf("\033[H\033[2J");
      nsminfo2();
      sleep(interval);
    }
  } else {
    nsminfo2();
  }
}
#endif /* NSMD2INFO */
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
