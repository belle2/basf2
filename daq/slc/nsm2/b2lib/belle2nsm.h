
#ifndef __belle2nsm_h__
#define __belle2nsm_h__

#include <nsm2/nsm2.h>

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

int b2nsm_addincpath(const char* path);
const char* b2nsm_nodename(int nodeid);
int b2nsm_nodeid(const char* nodename);
int b2nsm_nodepid(const char* nodename);
int b2nsm_loghook(NSMmsg* msg, NSMcontext* nsmc);
void b2nsm_checkpoint(NSMcontext* nsmc, int val);
int b2nsm_debuglevel(int val);
void b2nsm_logging(FILE* fp);
void b2nsm_logging2(FILE* fp, const char* prefix);
void b2nsm_context(NSMcontext* context);
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

void nsmlib_log(const char* fmt, ...);
#define b2nsm_printf nsmlib_log

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif

#endif /* __belle2nsm_h__ */
