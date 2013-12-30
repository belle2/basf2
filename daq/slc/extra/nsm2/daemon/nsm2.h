#ifndef __nsm2_h__
#define __nsm2_h__

/* -- version info */

#define NSM_PROTOCOL_VERSION 1915 /* protocol version 1.9.15 */
#define NSM_PACKAGE_VERSION  1918 /* package  version 1.9.18 */

/*
  20120723 1900 file created
  20121115 1903 queue based message sending starts working
  20121117 1904 queue based touchsys working for newclient
  20121218 1905 fix delclient
  (next is to implement hash)
  20130116 1906 hash implemented
  20130117 1907 hash fix1 (***hash not in network-byte order yet)
  20130117 1908 hash fix (***hash in network-byte order)
  20130219 1909 first version with usrcpymem (still many bugs)
  20130303 1910 alpha version release candidate
  20131218 1914 merged with Konno veresion
  20131219 1915 uid/gid for MEM shm
  20131222 1916 printlog infinite loop fix
  20131229 1917 no change in nsmd2, update in nsminfo2 and b2lib
  20131230 1918 libs: strerror fix, initnet fix, stdint fix, bridge fix
 */

/* -- DATA TYPES ----------------------------------------------------- */
#ifndef __nsm2_typedef_nonstdint__
#define __nsm2_typedef_nonstdint__
#include "stdint.h"
typedef uint8_t   byte8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
typedef int16_t   int16;
typedef int32_t   int32;
typedef int64_t   int64;
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
  uint16_t req;
  uint16_t seq;
  int16_t  node; /* source/destination nodeid, or (-1) if not an NSM client */
  uint8_t  npar;
  uint16_t len;
  int32_t  pars[256]; /* signed */
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
