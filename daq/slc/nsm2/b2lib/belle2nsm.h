/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* ---------------------------------------------------------------------- *\
   belle2nsm.h

   revision history can be found in belle2nsm.c
\* ---------------------------------------------------------------------- */

#ifndef __belle2nsm_h__
#define __belle2nsm_h__

#include "nsm2/nsm2.h"

#if defined(__cplusplus)
extern "C" {
#endif

void* b2nsm_getwrapptr();
int b2nsm_setwrapptr(void* ptr);
int b2nsm_addincpath(const char* path);
const char* b2nsm_nodename(int nodeid);
int b2nsm_nodeid(const char* nodename);
int b2nsm_nodepid(const char* nodename);
int b2nsm_nodeproc(const char* nodename);
const char* b2nsm_reqname(int reqid);
int b2nsm_reqid(const char* reqname);
int b2nsm_loghook(NSMmsg* msg, NSMcontext* nsmc);
void b2nsm_checkpoint(NSMcontext* nsmc, int val);
int b2nsm_debuglevel(int val);
FILE* b2nsm_logging(FILE* fp);
FILE* b2nsm_logging2(FILE* fp, const char* prefix);
NSMcontext* b2nsm_context(NSMcontext* context);
const char* b2nsm_strerror();
int b2nsm_callback(const char* name, NSMcallback_t callback);
int b2nsm_sendany(const char* node, const char* req, int npar, int32_t* pars,
                  int len, const char* datp, const char* caller);
int b2nsm_sendreq(const char* node, const char* req, int npar, int32_t* pars);

int b2nsm_ok(NSMmsg* msg, const char* newstate, const char* fmt, ...);
int b2nsm_error(NSMmsg* msg, const char* fmt, ...);
int b2nsm_readmem(void* buf, const char* dat, const char* fmt, int rev);
int b2nsm_statmem(const char* dat, char* fmtbuf, int buflen);
void* b2nsm_openmem(const char* dat, const char* fmt, int rev);
void* b2nsm_allocmem(const char* dat, const char* fmt, int rev, float cycle);
int b2nsm_flushmem(const void* ptr, int siz);
int b2nsm_wait(float timeout);
NSMcontext* b2nsm_init2(const char* nodename, int usesig, const char* host,
                        int port, int shmkey);
NSMcontext* b2nsm_init(const char* nodename);
int b2nsm_term();

void nsmlib_log(const char* fmt, ...);
#define b2nsm_printf nsmlib_log

#if defined(__cplusplus)
}
#endif

#endif /* __belle2nsm_h__ */
