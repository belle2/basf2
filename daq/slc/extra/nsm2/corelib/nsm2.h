#ifndef __nsm2_h__
#define __nsm2_h__

/* -- version info */

#define NSM_PROTOCOL_VERSION 1915 /* protocol version 1.9.15 */
#define NSM_PACKAGE_VERSION  1915 /* package  version 1.9.15 */

/*
  2012.07.23 1.9.00 --- file created
  2012.11.15 1.9.03 --- queue based message sending starts working
  2012.11.17 1.9.04 --- queue based touchsys working for newclient
  2012.12.18 1.9.05 --- fix delclient
  (next is to implement hash)
  2013.01.16 1.9.06 --- hash implemented
  2013.01.17 1.9.07 --- hash fix1 (***hash not in network-byte order yet)
  2013.01.17 1.9.08 --- hash fix (***hash in network-byte order)
  2013.02.19 1.9.09 --- first version with usrcpymem (still many bugs)
  2013.03.03 1.9.10 --- alpha version release candidate
  2013.12.18 1.9.14 --- merged with Konno veresion
  2013.12.19 1.9.15 --- uid/gid for MEM shm
 */

/* -- DATA TYPES ----------------------------------------------------- */
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

#ifndef __nsm2_typedef_context__
#define __nsm2_typedef_context__
struct NSMcontext_struct;
typedef struct NSMcontext_struct NSMcontext;
#endif /* nsm2_typedef_context */

/* request offset */
#define NSMREQ_FIRST       (0x1000)

/* no error */
#define NSMENOERR    (0)

/* unexpected system or internal errors */
#define NSMEALLOC      (-101)
#define NSMESOCKET     (-102)
#define NSMESOCKDGRAM  (-103)
#define NSMEGIFCONF    (-104)
#define NSMEGIFFLAGS   (-105)
#define NSMESOCKREUSE  (-106)
#define NSMESOCKSNDBUF (-107)
#define NSMESOCKRCVBUF (-108)
#define NSMERDSELECT   (-109)
#define NSMERDUID      (-110)
#define NSMENOPIPE     (-111)
#define NSMESELECT     (-112)
#define NSMETIMEOUT    (-113)
#define NSMEWRITE      (-114)
#define NSMECLOSED     (-115)
#define NSMEPIPEREAD   (-116)
#define NSMEPIPEWRITE  (-117)
#define NSMEMAXRETRY   (-118)
#define NSMEUNEXPECTED (-119)

/* possible errors by user parameters */
#define NSMENOMASTER  (-1) /* there is no master nsmd yet */
#define NSMEINVNAME   (-2) /* invalid name (length or character) */
#define NSMEINVPAR    (-3) /* invalid parameter(s) */
#define NSMENODEEXIST (-4) /* node already exist */
#define NSMEFULNODE   (-5) /* no more NSM node */
#define NSMENODEST    (-6) /* destination node is gone */
#define NSMEINVFMT    (-7) /* invalid data format */
#define NSMEMEMEXIST  (-8) /* data already exists */
#define NSMENOMOREMEM (-9) /* no more data area */
#define NSMEOPENED    (-10) /* already opened */
#define NSMENODENAME  (-11)
#define NSMENODELONG  (-12)
#define NSMEHOSTNAME  (-13)
#define NSMEALREADYP  (-14)
#define NSMEALREADYS  (-15)
#define NSMEALREADYH  (-16)
#define NSMENOIF      (-17)
#define NSMENONSMD    (-18)
#define NSMENOUID     (-19)
#define NSMERDCLOSE   (-20)
#define NSMEACCESS    (-21)
#define NSMESHMGETSYS (-22)
#define NSMESHMATSYS  (-23)
#define NSMESHMGETMEM (-24)
#define NSMESHMATMEM  (-25)
#define NSMENOINIT    (-26)
#define NSMEPERM      (-27) /* anonymous node can't send */
#define NSMEINVDATA   (-28) /* len==0 xor data==0 */
#define NSMEINVFUNC   (-29)
#define NSMEMAXFUNC   (-30)
#define NSMENOMEM     (-31) /* data does not exist (openmem) */
#define NSMEBADFMT    (-32) /* data format is inconsistent (openmem) */
#define NSMEOLDREV    (-33) /* data revision is old (openmem) */
#define NSMENEWREV    (-34) /* data revision is new (openmem) */
#define NSMESHMUID    (-35) /* given uid does not exist */
#define NSMEUIDPERM   (-36) /* given uid permission denied */
#define NSMESHMGID    (-37) /* given gid does not exist */
#define NSMEGIDPERM   (-38) /* given gid permission denied */

/* NSMmsg (in host byte order) */
typedef struct {
  uint16 req;
  uint16 seq;
  uint16 node; /* source/destination nodeid, or (-1) if not an NSM client */
  byte8  npar;
  uint16 len;
  uint32 pars[256];
  const char* datap;
} NSMmsg;

typedef void (*NSMcallback_t)(NSMmsg* msg, NSMcontext* nsmc);
typedef void (*NSMfunc_t)(NSMmsg* msg, NSMcontext* nsmc);

/* -- FUNCTIONS ------------------------------------------------------ */



#endif /* __nsm2_h__ */

/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
