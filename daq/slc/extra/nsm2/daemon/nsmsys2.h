/*
  nsmsys2.h

  This file may be included by both C and C++ programs.
*/

#ifndef __nsmsys2_h__
#define __nsmsys2_h__

#include <netinet/in.h> /* for struct sockaddr_in, sockaddr */

#define NSM2_PORT 8120 /* chosen as it corresponds to 0x2012
        (2012 is the year started writing NSM2) */
#define NSM2_MEMSIZ   (4096*1024)

#define NSMENV_HOST      "NSM2_HOST"
#define NSMENV_PORT      "NSM2_PORT"
#define NSMENV_SHMKEY    "NSM2_SHMKEY"

#define NSMDENV_USER     "NSMD2_USER"
#define NSMDENV_GROUP    "NSMD2_GROUP"
#define NSMDENV_DEBUG    "NSMD2_DEBUG"
#define NSMDENV_LOGDIR   "NSMD2_LOGDIR"
#define NSMDENV_PRIORITY "NSMD2_PRIORITY"
#define NSMDENV_TCPBUF   "NSMD2_TCPBUF"

#ifndef __nsm2_typedef_nonstdint__
#define __nsm2_typedef_nonstdint__
typedef unsigned char      byte8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned long long uint64;
typedef signed   short     int16;
typedef signed   int       int32;
typedef signed   long long int64;
#endif /* nsm2_typedef_nonstdint */

#ifndef __nsm2_typedef_sockad__
#define __nsm2_typedef_sockad__
typedef struct sockaddr    SOCKAD;
typedef struct sockaddr_in SOCKAD_IN;
#endif /* nsms2_typedef */

#define NSMSYS_NAME_SIZ 31
#define NSMSYS_MAX_NOD 1024
#define NSMSYS_MAX_REF 4096
#define NSMSYS_MAX_REG 8192
#define NSMSYS_MAX_DAT 1024
#define NSMSYS_MAX_CON 1024
#define NSMSYS_MAX_REQ 1024
#define NSMSYS_MAX_HASH 2039 /* prime number, about twice of MAX_NOD etc */
#define NSMSYS_MAX_SCH 256
#define NSMSYS_DNAM_SIZ 31
#define NSMSYS_DFMT_SIZ 63
#define NSMSYS_LOGDIR_SIZ  200
#define NSMSYS_LOGNAME_SIZ 256

/*
  udp message size is limited by MTU,
  tcp message size is limited by length in uint16_t and npar in byte8
*/
#define NSM_MTUSIZ     1500
#define NSM_IPHEADSIZ    20 /* optionally 24 */
#define NSM_UDPHEADSIZ    8
#define NSM_UDPMSGSIZ  1472
#define NSM_UDPDATSIZ  1464 /* header part */

#define NSM_TCPHEADSIZ 16 /* header */
#define NSM_TCPPARMAX 256 /* par in uint32_t */
#define NSM_TCPDATSIZ 65536 /* max size by uint16_t */
#define NSM_TCPBUFSIZ (256*4 + NSM_TCPDATSIZ) /* par + data */
#define NSM_TCPMSGSIZ (16 + NSM_TCPBUFSIZ) /* header + par + data */
#define NSM_TCPTHRESHOLD (16 + 256*4) /* at least header + par */

#define NSM_DATCHUNKSIZ 1464 /* MTU - UDPHEAD - udpbuf.head */
#define NSM_DATMAXCHUNK 44
#define NSM_DATMAXSIZ (NSM_DATCHUNKSIZ*NSM_DATMAXCHUNK) /* 64416 in uint16_t */

#define NSMD_MAXINVFMT  32
#define NSMD_TIMINVFMT  3600

/* NSMudpbuf (in network byte order) */
typedef struct {
  uint8_t  req;
  uint8_t  npar; /* num of parameters of int32_t in dat area */
  uint16_t seq;
  uint16_t par1;
  uint16_t par2;
  char   dat[NSM_UDPDATSIZ];
} NSMudpbuf;

/* NSMtcphead (in network byte order) */
typedef struct {
  uint16_t req;
  uint16_t seq;
  uint16_t src;
  uint16_t dest;
  uint8_t  opt;
  uint8_t  npar;
  uint16_t len;
  uint32_t from; // source IP, only when the message has to be forwarded
} NSMtcphead;

/* NSMDmsg (in host byte order) */
typedef struct {
  uint16_t req;
  uint16_t seq;
  uint16_t src;   // source      NSM nodeid, or (-1) if not an NSM client
  uint16_t dest;  // destination NSM nodeid, or (-1) if not an NSM client
  uint8_t  opt;
  uint8_t  npar;
  uint16_t len;
  uint32_t from;  // source IP address in network byte order
  uint32_t pars[256];
  const char* datap;
} NSMdmsg;

/* NSM messsage header is designed to minimize the header size
   max message length for UDP is 1468 byte = 367 32-bit-word */

/* The list of NSMDCMD_xxxx below should be in the same order as nsmd_reqlist
   which is implemented as an array instead of linear search list */

#define NSMCMD_FIRST       (0xff00)
#define NSMCMD_LAST        (0xff1c) /* last of UDP */

#define NSMCMD_NEWCLIENT   (NSMCMD_FIRST + 0x00)
#define NSMCMD_NEWCLIENTOB (NSMCMD_FIRST + 0x01)
#define NSMCMD_NEWMASTER   (NSMCMD_FIRST + 0x02)
#define NSMCMD_DELCLIENT   (NSMCMD_FIRST + 0x03)
#define NSMCMD_ALLOCMEM    (NSMCMD_FIRST + 0x04)
#define NSMCMD_FREEMEM     (NSMCMD_FIRST + 0x05)
#define NSMCMD_FLUSHMEM    (NSMCMD_FIRST + 0x06)
#define NSMCMD_REQCPYMEM   (NSMCMD_FIRST + 0x07)
#define NSMCMD_SYSCPYMEM   (NSMCMD_FIRST + 0x08)
#define NSMCMD_OPENMEM     (NSMCMD_FIRST + 0x09)
#define NSMCMD_CLOSEMEM    (NSMCMD_FIRST + 0x0a)
#define NSMCMD_DUMPNODE    (NSMCMD_FIRST + 0x0b)
#define NSMCMD_DUMPALLOC   (NSMCMD_FIRST + 0x0c)
#define NSMCMD_DUMPDATA    (NSMCMD_FIRST + 0x0d)
#define NSMCMD_DUMPCONN    (NSMCMD_FIRST + 0x0e)
#define NSMCMD_DUMPINFO    (NSMCMD_FIRST + 0x0f)
#define NSMCMD_DEBUGFLAG   (NSMCMD_FIRST + 0x10)
#define NSMCMD_KILLDAEMON  (NSMCMD_FIRST + 0x11)
#define NSMCMD_CLEANUP     (NSMCMD_FIRST + 0x12)
#define NSMCMD_TOUCHMEM    (NSMCMD_FIRST + 0x13)
#define NSMCMD_PING        (NSMCMD_FIRST + 0x14)
#define NSMCMD_PONG        (NSMCMD_FIRST + 0x15)
#define NSMCMD_READY       (NSMCMD_FIRST + 0x16)
#define NSMCMD_NOP         (NSMCMD_FIRST + 0x17)
#define NSMCMD_NEWREQ      (NSMCMD_FIRST + 0x18)
#define NSMCMD_DELREQ      (NSMCMD_FIRST + 0x19)

#define NSMCMD_UDPOFFSET   (0xff00)
#define NSMCMD_NEWDAEMON   (NSMCMD_FIRST + 0x1a)  /* for UDP */
#define NSMCMD_ACKDAEMON   (NSMCMD_FIRST + 0x1b)  /* for UDP */
#define NSMCMD_USRCPYMEM   (NSMCMD_FIRST + 0x1c)  /* for UDP */


typedef struct {
  char  name[NSMSYS_NAME_SIZ + 1]; /* 32 byte, *name == 0 if unavailable */
  int16_t rsvi16;
  int16_t noddat; /* sys.dat index or -1 if unavailable */
  int16_t nodref; /* sys.ref index or -1 if unavailable */
  int16_t nodreg; /* sys.reg index or -1 if unavailable */
  int32_t rsvi32;
  int32_t ipaddr; /* ip address    or -1 if unavailable */
  int32_t nodpid; /* process id */
  int32_t noduid; /* uid */
  int64_t nodtim; /* creation time */
} NSMnod; /* total 56 byte */

typedef struct {
  char  name[NSMSYS_NAME_SIZ + 1]; /* 32 byte, *name == 0 if unavailable */
  int16_t code;
  int16_t regid;
} NSMreq; /* total 40 byte */

typedef struct {
  int16_t regnod;
  int16_t regreq;
  int16_t nodnext;
  int16_t reqnext;
} NSMreg; /* total 8 byte */

typedef struct {
  int32_t refrsv;
  int16_t refnod; /* sys.nod index */
  int16_t refdat; /* sys.dat index */
  int64_t reftim; /* creation time */
} NSMref; /* total 16 byte */

typedef struct {
  char   dtnam[NSMSYS_DNAM_SIZ + 1]; /* 32 byte */
  char   dtfmt[NSMSYS_DFMT_SIZ + 1]; /* 64 byte */
  int16_t  dtrev; /* data revision */
  int16_t  owner;
  int16_t  anext; /* next sys.dat index or -1 if last */
  /* anext linked list is an address ascending list which is needed to
     find a free space for a new allocation */
  int16_t  nnext; /* next sys.dat index of the same node, or -1 if last */
  /* nnext linked list is needed to find all the data belong to the same
     node */
  uint16_t dtsiz; /* size up to 65120 (=1480*44), or 0 if unavailable */
  uint16_t dtref; /* number of reference count */
  uint16_t dtcyc; /* update cycle in 10msec unit, up to about 10 min */

  int16_t  dtrsv;
  int32_t  dtpos; /* mem.mem index (up to NSM2_MEMSIZ), or -1 if unavailable */
  int32_t  rsv32;
  int64_t  dttim; /* creation time in 10msec unit */
} NSMdat; /* total 128 byte */

typedef struct {
  int16_t  disid;
  int16_t  disnod;
  int32_t  discnt; /* update counter */
  int64_t  distim; /* time to distribute in 10msec unit */
} NSMdat_snd;

typedef struct {
  int32_t  rcvcnt;
  int32_t  rcvdt1; /* delta time between newest update and dttim */
  int16_t  dtddt[44]; /* delta time between newest and each chunk */
} NSMdat_rcv;



#define NSMCON_NON  (-1)
#define NSMCON_UDP  0
#define NSMCON_TCP  1
#define NSMCON_OUT  2

typedef struct {
  int32_t sock;
  int32_t pid;
  int32_t nid;
  int16_t status; /* flag for invalid connection: 0=OK, 1=WAITING, 2=NG */
  int16_t ready;  /* flag for ready to become the master */
  /*
    (&udpcon = sys.con[NSMCON_UDP], &tcpcon = sys.con[NSMCON_TCP])
    udpcon.timstart for the start time of the daemon
    tcpcon.timstart for the start time of the NSM network
    udpcon.timevent when master/deputy/priority/generation modified
    tcpcon.timevent when nod/ref/dat modified
   */
  int64_t timstart; /* start time reported from remote */
  int64_t timevent; /* when status changed */
  SOCKAD_IN sockad; /* 16 bytes */
  int32_t icnt;  /* how many times recv */
  int32_t ocnt;  /* how many times sent */
  int64_t isiz;  /* how many bytes recv */
  int64_t osiz;  /* how many bytes sent */
  int64_t otim;  /* how much time (in us) select had to wait */
  int32_t osel;  /* how many times select had to wait */
  int32_t rsv32;
  uint8_t sigobs;   /* sigobs=1 if SIGRTMIN is not defined */
  uint8_t priority; /* between 0-100 */
  uint8_t rsv;
  char  name[13];  /* (only first 12 bytes, total 80 bytes */
} NSMcon;

typedef struct {
  int64_t when;   /* time_t */
  int32_t opt;    /* option */
  int16_t conid;  /* conid, to remove when connection closed */
  int16_t funcid; /* function to call */
} NSMsch;

typedef struct {
  /* first 4 words are common to both NSMmem and NSMsys */
  int32_t ipaddr;
  int32_t pid;
  int64_t timstart;
  int64_t timevent;
  char mem[NSM2_MEMSIZ];
} NSMmem;

typedef struct {
  /* first 4 words are common to both NSMmem and NSMsys */
  int32_t ipaddr;
  int32_t pid;
  int64_t timstart; /* creation time of NSMsys (may not by me) */
  int64_t timevent; /* last update of NSMsys */
  uint32_t netmask;
  int16_t sock_updated;  /* socket updated, but fdset not updated yet */
  int16_t afirst; /* set and distributed by master */
  int16_t nnod; /* set and distributed by master */
  int16_t nref; /* set and distributed by master */
  int16_t ndat; /* set and distributed by master */
  int16_t nreq; /* set and distributed by master */
  NSMnod nod[NSMSYS_MAX_NOD]; /* set and distributed by master */
  NSMref ref[NSMSYS_MAX_REF]; /* set and distributed by master */
  NSMdat dat[NSMSYS_MAX_DAT]; /* set and distributed by master */
  NSMreq req[NSMSYS_MAX_REQ]; /* set and distributed by master */
  NSMreg reg[NSMSYS_MAX_REG]; /* set and distributed by master */

  /* hash tables (+1 for alignment as MAX_HASH is not even */
  int32_t nodhash[NSMSYS_MAX_HASH + 1]; /* set and distributed by master */
  int32_t dathash[NSMSYS_MAX_HASH + 1]; /* set and distributed by master */
  int32_t reqhash[NSMSYS_MAX_HASH + 1]; /* set and distributed by master */

  NSMcon con[NSMSYS_MAX_CON];
  NSMsch sch[NSMSYS_MAX_SCH];
  int32_t ncon;
  int32_t nsch;
  int16_t conid[NSMSYS_MAX_NOD];  /* nod => con mapping */
  int16_t ireg_free; /* cache to make it easier to find new reg slot */
  int16_t nsnd;
  int16_t rsv16[2];
  NSMdat_snd snd[NSMSYS_MAX_DAT];
  NSMdat_rcv rcv[NSMSYS_MAX_DAT];

  int16_t generation;
  int16_t master;
  int16_t deputy;
  int16_t ready;

  /* copy of global variables for reference (not for use by nsmd) */
  int16_t version;  /* nsmd version */
  int16_t protocol; /* protocol version */
  int16_t priority;
  int16_t debug;
  char    logfile[NSMSYS_LOGNAME_SIZ];
} NSMsys;

/* NSMDtcpq for queuing of tcp-write messages */
typedef struct nsmd_tcpq {
  int     conid;
  int     req;    /* host-byte-order: redundant info also in buf */
  int     npar;   /* host-byte-order: redundant info also in buf */
  int     msglen; /* host-byte-order: redundant info also in buf */
  struct nsmd_tcpq* prevp;
  struct nsmd_tcpq* nextp;
  int     pos;
  const char*  datap;
  char    buf[NSM_TCPTHRESHOLD];  /* network-byte-order */
} NSMDtcpq;

#endif /* __nsmsys2_h__ */

/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
