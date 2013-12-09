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

#define NSMENV_HOST   "NSM2_HOST"
#define NSMENV_PORT   "NSM2_PORT"
#define NSMENV_SHMKEY "NSM2_SHMKEY"

#define NSMDENV_DEBUG  "NSMD2_DEBUG"
#define NSMDENV_LOGDIR "NSMD2_LOGDIR"
#define NSMDENV_PRIORITY "NSMD2_PRIORITY"
#define NSMDENV_TCPBUF   "NSMD2_TCPBUF"

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
  tcp message size is limited by length in uint16 and npar in byte8
*/
#define NSM_MTUSIZ     1500
#define NSM_UDPHEADSIZ    8
#define NSM_UDPMSGSIZ  1492
#define NSM_UDPDATSIZ  1484 /* header part */

#define NSM_TCPHEADSIZ 16 /* header */
#define NSM_TCPPARMAX 256 /* par in uint32 */
#define NSM_TCPDATSIZ 65536 /* max size by uint16 */
#define NSM_TCPBUFSIZ (256*4 + NSM_TCPDATSIZ) /* par + data */
#define NSM_TCPMSGSIZ (16 + NSM_TCPBUFSIZ) /* header + par + data */
#define NSM_TCPTHRESHOLD (16 + 256*4) /* at least header + par */

#define NSM_DATCHUNKSIZ 1484 /* MTU - UDPHEAD - udpbuf.head */
#define NSM_DATMAXCHUNK 44
#define NSM_DATMAXSIZ   (NSM_DATCHUNKSIZ*NSM_DATMAXCHUNK) /* 65120 in uint16 */

#define NSMD_MAXINVFMT  32
#define NSMD_TIMINVFMT  3600

/* NSMudpbuf (in network byte order) */
typedef struct {
  byte8  req;
  byte8  npar; /* num of parameters of int32 in dat area */
  uint16 seq;
  uint16 par1;
  uint16 par2;
  char   dat[NSM_UDPDATSIZ];
} NSMudpbuf;

/* NSMtcphead (in network byte order) */
typedef struct {
  uint16 req;
  uint16 seq;
  uint16 src;
  uint16 dest;
  byte8  opt;
  byte8  npar;
  uint16 len;
  uint32 from; // source IP, only when the message has to be forwarded
} NSMtcphead;

/* NSMDmsg (in host byte order) */
typedef struct {
  uint16 req;
  uint16 seq;
  uint16 src;   // source      NSM nodeid, or (-1) if not an NSM client
  uint16 dest;  // destination NSM nodeid, or (-1) if not an NSM client
  byte8  opt;
  byte8  npar;
  uint16 len;
  uint32 from;  // source IP address in network byte order
  uint32 pars[256];
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
  int16 rsvi16;
  int16 noddat; /* sys.dat index or -1 if unavailable */
  int16 nodref; /* sys.ref index or -1 if unavailable */
  int16 nodreg; /* sys.reg index or -1 if unavailable */
  int32 rsvi32;
  int32 ipaddr; /* ip address    or -1 if unavailable */
  int32 nodpid; /* process id */
  int32 noduid; /* uid */
  int64 nodtim; /* creation time */
} NSMnod; /* total 56 byte */

typedef struct {
  char  name[NSMSYS_NAME_SIZ + 1]; /* 32 byte, *name == 0 if unavailable */
  int16 code;
  int16 regid;
} NSMreq; /* total 40 byte */

typedef struct {
  int16 regnod;
  int16 regreq;
  int16 nodnext;
  int16 reqnext;
} NSMreg; /* total 8 byte */

typedef struct {
  int32 refrsv;
  int16 refnod; /* sys.nod index */
  int16 refdat; /* sys.dat index */
  int64 reftim; /* creation time */
} NSMref; /* total 16 byte */

typedef struct {
  char   dtnam[NSMSYS_DNAM_SIZ + 1]; /* 32 byte */
  char   dtfmt[NSMSYS_DFMT_SIZ + 1]; /* 64 byte */
  int16  dtrev; /* data revision */
  int16  owner;
  int16  anext; /* next sys.dat index or -1 if last */
  /* anext linked list is an address ascending list which is needed to
     find a free space for a new allocation */
  int16  nnext; /* next sys.dat index of the same node, or -1 if last */
  /* nnext linked list is needed to find all the data belong to the same
     node */
  uint16 dtsiz; /* size up to 65120 (=1480*44), or 0 if unavailable */
  uint16 dtref; /* number of reference count */
  uint16 dtcyc; /* update cycle in 10msec unit, up to about 10 min */

  int16  dtrsv;
  int32  dtpos; /* mem.mem index (up to NSM2_MEMSIZ), or -1 if unavailable */
  int32  rsv32;
  int64  dttim; /* creation time in 10msec unit */
} NSMdat; /* total 128 byte */

typedef struct {
  int16  disid;
  int16  disnod;
  int32  discnt; /* update counter */
  int64  distim; /* time to distribute in 10msec unit */
} NSMdat_snd;

typedef struct {
  int32  rcvcnt;
  int32  rcvdt1; /* delta time between newest update and dttim */
  int16  dtddt[44]; /* delta time between newest and each chunk */
} NSMdat_rcv;



#define NSMCON_NON  (-1)
#define NSMCON_UDP  0
#define NSMCON_TCP  1
#define NSMCON_OUT  2

typedef struct {
  int32 sock;
  int32 pid;
  int32 nid;
  int16 status; /* flag for invalid connection: 0=OK, 1=WAITING, 2=NG */
  int16 ready;  /* flag for ready to become the master */
  /*
    (&udpcon = sys.con[NSMCON_UDP], &tcpcon = sys.con[NSMCON_TCP])
    udpcon.timstart for the start time of the daemon
    tcpcon.timstart for the start time of the NSM network
    udpcon.timevent when master/deputy/priority/generation modified
    tcpcon.timevent when nod/ref/dat modified
   */
  int64 timstart; /* start time reported from remote */
  int64 timevent; /* when status changed */
  SOCKAD_IN sockad; /* 16 bytes */
  int32 icnt;  /* how many times recv */
  int32 ocnt;  /* how many times sent */
  int64 isiz;  /* how many bytes recv */
  int64 osiz;  /* how many bytes sent */
  int64 otim;  /* how much time (in us) select had to wait */
  int32 osel;  /* how many times select had to wait */
  int32 rsv32;
  byte8 sigobs;   /* sigobs=1 if SIGRTMIN is not defined */
  byte8 priority; /* between 0-100 */
  byte8 rsv;
  char  name[13];  /* (only first 12 bytes, total 80 bytes */
} NSMcon;

typedef struct {
  int64 when;   /* time_t */
  int32 opt;    /* option */
  int16 conid;  /* conid, to remove when connection closed */
  int16 funcid; /* function to call */
} NSMsch;

typedef struct {
  /* first 4 words are common to both NSMmem and NSMsys */
  int32 ipaddr;
  int32 pid;
  int64 timstart;
  int64 timevent;
  char mem[NSM2_MEMSIZ];
} NSMmem;

typedef struct {
  /* first 4 words are common to both NSMmem and NSMsys */
  int32 ipaddr;
  int32 pid;
  int64 timstart; /* creation time of NSMsys (may not by me) */
  int64 timevent; /* last update of NSMsys */
  uint32 netmask;
  int16 dirty;  /* connection is updated, but fdset is not updated yet */
  int16 afirst; /* set and distributed by master */
  int16 nnod; /* set and distributed by master */
  int16 nref; /* set and distributed by master */
  int16 ndat; /* set and distributed by master */
  int16 nreq; /* set and distributed by master */
  NSMnod nod[NSMSYS_MAX_NOD]; /* set and distributed by master */
  NSMref ref[NSMSYS_MAX_REF]; /* set and distributed by master */
  NSMdat dat[NSMSYS_MAX_DAT]; /* set and distributed by master */
  NSMreq req[NSMSYS_MAX_REQ]; /* set and distributed by master */
  NSMreg reg[NSMSYS_MAX_REG]; /* set and distributed by master */

  /* hash tables (+1 for alignment as MAX_HASH is not even */
  int32 nodhash[NSMSYS_MAX_HASH + 1]; /* SYSPOS set and distributed by master */
  int32 dathash[NSMSYS_MAX_HASH + 1]; /* SYSPOS set and distributed by master */
  int32 reqhash[NSMSYS_MAX_HASH + 1]; /* SYSPOS set and distributed by master */

  NSMcon con[NSMSYS_MAX_CON];
  NSMsch sch[NSMSYS_MAX_SCH];
  int32 ncon;
  int32 nsch;
  int16 conid[NSMSYS_MAX_NOD];  /* nod => con mapping */
  int16 ireg_free; /* cache to make it easier to find new reg slot */
  int16 nsnd;
  int16 rsv16[2];
  NSMdat_snd snd[NSMSYS_MAX_DAT];
  NSMdat_rcv rcv[NSMSYS_MAX_DAT];

  int16 generation;
  int16 master;
  int16 deputy;
  int16 ready;


  /* copy of global variables for reference (not for use by nsmd) */
  int16 version;  /* nsmd version */
  int16 priority;
  int16 debug;
  int16 nofork;
  char  logfile[NSMSYS_LOGNAME_SIZ];
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
