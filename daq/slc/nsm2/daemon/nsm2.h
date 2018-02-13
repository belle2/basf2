#ifndef __nsm2_h__
#define __nsm2_h__

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

/* -- version info */

#define NSM_PROTOCOL_VERSION 1944 /* protocol version */
#define NSM_PACKAGE_VERSION  1958 /* package  version */

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
  20140104 1919 b2libs: b2nsm_ok text, nsmd2: disid fix
  20140105 1920 nsminfo2 from nsmd2
  20140106 1921 destroyconn fix, etc
  20140107 1922 fixing the case when both master/deputy are killed
  20140107 1923 fixing priority passing
  20140114 1924 rewrite of nsmd_do_newmaster
  20140117 1924 nsmd2 check error before going background, many crucial bug
                fixes on nsmd master/deputy switching, color nsminfo2
  20140124 1925 anonymous node
  20140304 1926 new parser, 20 byte smaller chunk size for IP header
  20140305 1927 freeq fix for sending a long packet
  20140305 1928 fix to 1927, DoS vulnerability test, still far from OK
  20140306 1929 no fprintf for DoS vulnerability trial tests, in vain
  20140306 1930 restore logfp and cleanup (but still write instead of fwrite)
  20140516 1931 corelib: ip address from shm if not specified
  20140614 1932 corelib: sprintf ip address if host is missing
  20140614 1933 corelib: use nodhash for nsmlib_nodeid
  20140902 1934 static bsizbuf pollution fix, broken tcprecv debug
  20140902 1935 memset fix
  20140903 1936 debug message fix
  20140903 1937 nsmparse fix (see nsmparse.c)
  20140917 1938 newclient error return fix / shm cleanup fix
  20140917 1939 skip revision check by -1
  20140921 1940 bytes in nsmparse_t, flushmem, less DBG in nsmd2
  20140922 1941 nsmget is added
  20140922 1942 nodtim fix
  20150520 1943 destroyconn fix [for anonymous and for sys.ready]
  20150521 1944 new protocol version, master recoonect fix
  20150808 1945 updates for PNNL summer school tutorial
  20160420 1946 suppress debug output of nsmd2 and b2lib
  20170613 1947 nsmd2 update for USRCPYMEM protection
  20170927 1948 nsmd2 touchsys pos fix, log cleanup for send/recv
  20170929 1949 nsmd2 ackdaemon fix
  20170102 1950 nsmd2 ackdaemon more fix
  20171002 1951 nsmd2 destroyconn/ackdaemon fix
  20171002 1952 nsmd2 recv buffer to global
  20180111 1953 debug orphan node
  20180118 1954 nsmlib fix on select timeout, conid fix, message cleanup
  20180118 1955 put more priority to less frequent messages
  20180120 1956 b2nsm_wait to accept usec precision time
  20180121 1957 b2nsm_nodename added
  20180124 1958 smarter nsmd2 debug messages
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
/* #define NSMEMAXRETRY   (-118) */
#define NSMEDATID      (-118)
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
#define NSMEBADREV    (-33) /* data revision is inconsistent (openmem) */
#define NSMEPARSE     (-34) /* data format parse error (openmem) */
#define NSMECONNECT   (-35) /* connection error */
#define NSMEINVPTR    (-36) /* invalid data pointer */

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

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif

#endif /* __nsm2_h__ */

/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
