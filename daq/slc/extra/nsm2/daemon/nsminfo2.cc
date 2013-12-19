// ----------------------------------------------------------------------
// -- nsminfo2.cc
//
// 20131218  xforce option [from nsmd 1913]
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

#include "nsm2.h"
#include "nsmsys2.h"

// -- global variables --------------------------------------------------
// ----------------------------------------------------------------------
int nsmd_port = NSM2_PORT;
int nsmd_shmkey = -1; /* == nsmd_port if -1 */

NSMsys* nsmd_sysp = 0;
NSMmem* nsmd_memp = 0;
static int xforce = 0;
static int xverbose = 0;

// -- macros ------------------------------------------------------------
// ----------------------------------------------------------------------

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
// -- init --------------------------------------------------------------
void
nsminfo_init(int nsmd_shmkey)
{
  int id;
  if ((id = shmget(nsmd_shmkey, sizeof(*nsmd_sysp), 0555)) < 0) {
    if (errno = ENOENT) {
      printf("Cannot open SYS shared memory with key=%d.  Is nsmd running?\n",
             nsmd_shmkey);
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
  if ((id = shmget(nsmd_shmkey + 1, sizeof(*nsmd_memp), 0777)) < 0) {
    if (errno = ENOENT) {
      printf("Cannot open MEM shared memory with key=%d. %s\n",
             nsmd_shmkey + 1, "Something is inconsistent");
    } else {
      printf("shmkey (mem) = %d\n", nsmd_shmkey + 1);
      perror("shmget (mem):");
    }
    exit(1);
  }
  nsmd_memp = (NSMmem*)shmat(id, 0, SHM_RDONLY);
  if (nsmd_memp == (NSMmem*) - 1) {
    printf("shmkey (mem) = %d\n", nsmd_shmkey + 1);
    perror("shmat (mem):");
    exit(1);
  }
}
// -- hoststr -----------------------------------------------------------
const char*
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
    sprintf(hostp, "%d.%d.%d.%d",
            (iph >> 24) & 255, (iph >> 16) & 255, (iph >> 8) & 255, (iph >> 0) & 255);
  }

  return hostp;
}
// -- timestr -----------------------------------------------------------
const char*
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
      sprintf(timep, "%ds", dt);
    } else if (dt < 60 * 60) {
      sprintf(timep, "%dm%02ds", dt / 60, dt % 60);
    } else if (dt < 60 * 60 * 24) {
      sprintf(timep, "%dh%02dm", (dt / 60) / 60, (dt / 60) % 60);
    } else {
      sprintf(timep, "%dd%02dh", (dt / 3600) / 24, (dt / 3600) % 24);
    }
  } else {
    if (t == 0) return "unknown-time";

    tm tmbuf;
    localtime_r(&t, &tmbuf);
    sprintf(timep, "%04d.%02d.%02d-%02d:%02d:%02d",
            tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday,
            tmbuf.tm_hour, tmbuf.tm_min, tmbuf.tm_sec);
  }

  return timep;
}
// -- head --------------------------------------------------------------
void
nsminfo_head()
{
  NSMsys& sys = *nsmd_sysp;
  NSMmem& mem = *nsmd_memp;
  pid_t  pid    = sys.pid;
  time_t tstart = sys.timstart;
  time_t tevent = sys.timevent;
  int badmem = 0;
  NSMcon& master = sys.con[sys.master];
  NSMcon& udpcon = sys.con[NSMCON_UDP];
  NSMcon& tcpcon = sys.con[NSMCON_TCP];

  if (sys.master == NSMCON_NON) {
    printf("NSM network is not started yet\n");
  } else {
    printf("NSM network started at %s (%s ago)\n",
           nsminfo_timestr(tcpcon.timstart, 1),
           nsminfo_timestr(tcpcon.timstart));
  }
  printf("NSMD daemon started at %s (%s ago)\n",
         nsminfo_timestr(udpcon.timstart, 1),
         nsminfo_timestr(udpcon.timstart));

  printf("MASTER(%d) %s %s, DEPUTY(%d) %s %s, I'm %s nsmd\n",
         sys.master,
         sys.master == NSMCON_NON ? "is" : "at",
         sys.master == NSMCON_NON ? "missing" : nsminfo_hoststr(AddrMaster()),
         sys.deputy,
         sys.deputy == NSMCON_NON ? "is" : "at",
         sys.deputy == NSMCON_NON ? "missing" : nsminfo_hoststr(AddrDeputy()),
         sys.master == NSMCON_TCP ? "the MASTER" :
         sys.deputy == NSMCON_TCP ? "the DEPUTY" : "a MEMBER");

  if (sys.ipaddr != mem.ipaddr) {
    printf("inconsistent ipaddr: %s(sys) vs %s(mem)\n",
           nsminfo_hoststr(sys.ipaddr, 1), nsminfo_hoststr(mem.ipaddr, 1));
    badmem++;
  }
  if (pid != mem.pid) {
    printf("inconsistent process-id: %d vs %d\n", pid, mem.pid);
    badmem++;
  }
  if (tstart != mem.timstart) {
    printf("inconsistent start time: %d vs %d\n",
           (int)tstart, (int)mem.timstart);
    badmem++;
  }
  if (abs(tevent - mem.timevent) > 1) {
    printf("inconsistent time: %d vs %d\n",
           (int)tevent, (int)mem.timevent);
    badmem++;
  }
  if (badmem && ! xforce) {
    exit(1);
  }

  printf("NSMD shmkey = %d, ip = %s, pid = %d\n",
         nsmd_shmkey, nsminfo_hoststr(sys.ipaddr, 1),
         nsmd_sysp->pid);
  printf("     created %s ago, last updated %s ago%s\n",
         nsminfo_timestr(sys.timstart),
         nsminfo_timestr(sys.timevent),
         (time(0) - sys.timevent <= 2) ? " (up-to-date)" : "");
}
// -- conn --------------------------------------------------------------
void
nsminfo_conn(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;
  int ncon = all ? NSMSYS_MAX_CON : sys.ncon;

  for (int icon = NSMCON_OUT; icon < ncon; icon++) {
    NSMcon& con = sys.con[icon];
    char name[32];

    if (icon == NSMCON_TCP) {
      strcpy(name, "name=[TCP]");
    } else if (icon == NSMCON_UDP) {
      strcpy(name, "name=[BCAST]");
    } else if (ItsLocal(icon)) {
      //sprintf(name, "name=%s", sys->nod[sys->con[i].nid].name);
      sprintf(name, "nodeid=%d", con.nid);
    } else {
      sprintf(name, "host=%s", nsminfo_hoststr(AddrConn(icon)));
    }
    printf("CONN %-3.1d nid=%3d pid=%5d sock=%2d rtim=%-6.6s %s%s\n",
           icon,
           con.nid, con.pid, con.sock, nsminfo_timestr(con.timstart), name,
           (icon == sys.master) ? " M" : ((icon == sys.deputy) ? " D" : "")
          );
  }
}
// -- node --------------------------------------------------------------
void
nsminfo_node(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int inod = 0; inod < NSMSYS_MAX_NOD; inod++) {
    NSMnod& nod = sys.nod[inod];
    if (! nod.name[0]) continue;

    printf("NODE %-3.1d %-31.31s pid/uid=%-5.1d/%-5.1d @%s %d\n",
           inod, nod.name, (int32)ntohl(nod.nodpid), ntohl(nod.noduid),
           nsminfo_hoststr(nod.ipaddr), (int16)ntohs(nod.noddat));
  }
}
// -- req ---------------------------------------------------------------
void
nsminfo_req(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int ireq = 0; ireq < NSMSYS_MAX_REQ; ireq++) {
    NSMreq& req = sys.req[ireq];
    if (! req.code || ! req.name[0]) continue;

    int hash = nsmlib_hash(&sys, sys.reqhash, NSMSYS_MAX_HASH, req.name, 0);
    int reqpos = ntohl(sys.reqhash[hash]);
    NSMreq* reqp = (NSMreq*)SYSPTR(reqpos);

    printf("REQ  %-3.1d %-31.31s code=%04x hash=%d(%d,%04x)\n",
           ireq, req.name, ntohs(req.code), hash, reqpos,
           hash >= 0 && reqpos > 0 && reqpos < sizeof(sys) ? ntohs(reqp->code) : -1);
  }
}
// -- dat ---------------------------------------------------------------
void
nsminfo_dat(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int idat = 0; idat < NSMSYS_MAX_DAT; idat++) {
    NSMdat& dat = sys.dat[idat];
    if (dat.dtsiz == 0 && xverbose == 0) continue;
    if (xverbose) {
      printf("DAT  %-3.1d %-31.31s pos=%d sz=%d nod=%d rev=%d ref=%d fmt=%s\n",
             idat, dat.dtnam, ntohl(dat.dtpos), ntohs(dat.dtsiz),
             (int16)ntohs(dat.owner), ntohs(dat.dtrev), ntohs(dat.dtref),
             dat.dtfmt);
    } else {
      printf("DAT  %-3.1d %-31.31s sz=%d nod=%d rev=%d ref=%d fmt=%s\n",
             idat, dat.dtnam, ntohs(dat.dtsiz),
             (int16)ntohs(dat.owner), ntohs(dat.dtrev), ntohs(dat.dtref),
             dat.dtfmt);
    }
  }
}
// -- ref ---------------------------------------------------------------
void
nsminfo_ref(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;

  for (int iref = 0; iref < NSMSYS_MAX_DAT; iref++) {
    NSMref& ref = sys.ref[iref];
    if (ref.refnod == -1) continue;
    printf("REF  %-3.1d: %-3.1d => %-3.1d\n",
           iref, ntohs(ref.refnod), ntohs(ref.refdat));
  }
}
// -- foo ---------------------------------------------------------------
void
nsminfo_foo(int all = 0)
{
  NSMsys& sys = *nsmd_sysp;
  int anext = (int16)ntohs(sys.afirst);
  int amax = 1024;

  printf("alist = %d", anext);
  while (amax-- > 0 && anext != -1) {
    NSMdat& dat = sys.dat[anext];
    anext = (int16)ntohs(dat.anext);
    printf(" %d", anext);
  }
  printf("\n");
}
// -- main --------------------------------------------------------------
//
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  int port = -1;
  int shmkey = -1;

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

    if (strchr("ps", argv[1][1]) && ! *ap) {
      argc--, argv++;
      ap = argv[1];
    }

    switch (opt) {
      case 'p': port   = nsmd_atoi(ap, -1); break;
      case 's': shmkey = nsmd_atoi(ap, -1); break;
      default:
        printf("usage: nsminfo2 [options]\n");
        printf(" -p <port>   set port number.\n");
        printf(" -s <shmkey> set shmkey number.\n");
        exit(1);
    }
    argc--, argv++;
  }

  if (port < 0)   port   = nsmd_atoi(getenv(NSMENV_PORT));
  if (shmkey < 0) shmkey = nsmd_atoi(getenv(NSMENV_SHMKEY), -1);

  if (port > 0)    nsmd_port   = port;
  if (shmkey >= 0) nsmd_shmkey = shmkey;
  if (nsmd_shmkey < 0) nsmd_shmkey = nsmd_port;

  nsminfo_init(nsmd_shmkey);
  nsminfo_head();
  nsminfo_conn();
  nsminfo_node();
  nsminfo_req();
  nsminfo_dat();
  nsminfo_ref();

  nsminfo_foo();
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
