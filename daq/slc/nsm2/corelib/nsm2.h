/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef __nsm2_h__
#define __nsm2_h__

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

/* -- version info */

#define NSM_REQUIRED_VERSION  1999 /* required version */
#define NSM_LIBRARY_VERSION   2003 /* library version */
#define NSM_PACKAGE_VERSION   2006 /* package version */

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
  20150521 1944 new protocol version, master reconnect fix
  20150808 1945 updates for PNNL summer school tutorial
  20160420 1946 suppress debug output of nsmd2 and b2lib
  20170613 1947 nsmd2 update for USRCPYMEM protection
  20180326 1959 based on 1947, just change the handling of -o option
  20180327 1960 delconn/touchsys fix, better log, nsminfo2 hostname
  20180328 1961 limit syscpymem.pos in uint16 range
  20180329 1962 nsmlib.h fix for nsmlib_parsefile type
  20180404 1963 nsminfo2 no hostname resolving by default
  20180412 1964 one more nsminfo2 update on hostname cache
  20180417 1965 suppress "bad" in usrcpymem if not ready
  20180423 1966 destroyconn when write error at tcpwriteq
  20180430 1967 src/dest at command and lastmsg at delconn for debug
  20180502 1968 fix nsmd_dbg of 1967 printing all DBG messages
  20180504 1969 b2nsm_nodename restored from 1957
  20180515 1970 int16_t fix for owner in usrcpymem
  20180519 1971 tcprecv static buffer shift upon delcon
  20180521 1972 further debug of 1971
  20180523 1973 nsminfo2 usage print
  20180709 1974 nsmlib memory leak fix, etc, nsmd2 is rolled back to 1970
  20180711 1975 nsmlib update again
  20180801 1976 nsmd2 for better shm handling, b2nsm_term (experimental)
  20180802 1977 example update, shmget fix, pipe close, USRCPYMEM delay
  20180811 1978 fix prio, ready, forward, etc for master switch
  20180815 1979 nsmd2 fix conid on master, nsminfo2 -X fix
  20180815 1980 nsmd2 partial fix of node already exists
  20180815 1981 nsmd2 hopefully final fix of node already exists
  20180820 1982 nsmd2 kill HUP when connection is closed
  20180820 1983 nsmd2 tcprecv local conid shift upon delcon, c++example added
  20180822 1984 nsmd2 quick patch to avoid kill 0 HUP
  20180823 1985 nsmd2 debug for tcprecv assert
  20180824 1986 nsmd2 tcprecv debug continue, localtime => localtime_r
  20180826 1987 nsmd2 tcprecv debug done, nsmd_localtime_r, verbose log
  20180826 1988 nsmd2 code cleanup and suppress log for release version
  20180902 1989 c++example fix, simplest.cc is added
  20180927 1990 nsmd2 another orphan node fix, b2lib fixes
  20181009 1991 restoring 1970 and cope with 1978 gen/prio fix
  20181029 1992 double free bug fix
  20181029 1993 experimental: do not die in do_delclient bad ip
  20190415 1994 add: NSMESHMNOSYS, NSMESHMACCMEM, nsmc->errn
  20190524 1994 mod: NSMENODEST, shm handling, nodeproc, makefile
  20190524 1995 add: nsmstat2, fix: nsmd2 ocnt/osiz/otim
  20190529 1994 fix: nsmlib2.c null sysp handling
  20190529 1995 merge: fix in 1994
  20190530 1996 fix: nsmd2 int function must always return (for g++ 8.2.0 -O)
  20190724 1997 mod: version definition, nsminfo2 format, log filename
  20190903 1998 fix: nsmd2 momentary deputy, select timeout in tcprecv
  20190913 1999 mod: b2nsm_logging and b2nsm_context return previous value
  20191002 1999 mod: nsmsys2.h should not be needed by user
  20191123 1999 fix: nsm2sh send_vset
  20200121 1999 fix: call destroyconn upon new re-accept
  20200122 1999 fix: reuse tm in reopenlog
  20200122 1999 fix: bad tcphead
  20200127 2000 dbg: nsmstat2 crash
  20200314 2001 dbg: nsmstat2 crash (32-bit host), add: localhost support
  20200416 2002 add: killnsm2, mod: nsmd2.cc code cleanup
  20200520 2003 fix: bad tcphead by newclient, avoid already exist error
  20200520 2004 fix: close fd 0,1,2 before running in background
  20200522 2005 fix: allow udp socket == 0, eliminate log at startup with -b
  20200603 2006 fix: corelib nnod fix
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
#define NSMENODEST    (-6) /* destination node does not exist */
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
#define NSMESHMNOSYS  (-37) /* ESHMGETSYS and ENOENT */
#define NSMESHMACCES  (-38) /* ESHMGETMEM and EACCES */
#define NSMEEMPTYDEST (-39) /* empty string for destination node */
#define NSMENGMASTER  (-40) /* master nsmd2 temporarily unreachable */
#define NSMETIMEOUTW  (-41) /* timeout in sending to nsmd2 */
#define NSMETIMEOUTR  (-42) /* timeout in receiving from nsmd2 */
#define NSMENOSUCHREQ (-43) /* no such registered request */
#define NSMEBADHASH   (-44) /* broken internal hash table */
#define NSMENODEHERE  (-45) /* node already exist on the same host */

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
