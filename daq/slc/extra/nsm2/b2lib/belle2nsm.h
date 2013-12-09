
#include "nsm2.h"

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(__dummy_close_bracket_to_cheat_emacs_auto_indent)
}
#endif

NSMcontext* b2nsm_init2(const char* nodename, int usesig, const char* host,
                        int port, int shmkey);

NSMcontext* b2nsm_init(const char* nodename);
void b2nsm_context(NSMcontext* context);
const char* b2nsm_strerror();
int b2nsm_callback(const char* name, NSMcallback_t callback);
void* b2nsm_openmem(const char* dat, const char* fmt, int rev);
void* b2nsm_allocmem(const char* dat, const char* fmt, int rev, float cycle);
int b2nsm_sendreq(const char* node, const char* req, int npar, int* pars);
int b2nsm_sendreq_data(const char* node, const char* req, int npar, int* pars, int len, const char* datap);

int b2nsm_ok(NSMmsg* msg, const char* newstate, const char* fmt, ...);
int b2nsm_error(NSMmsg* msg, const char* fmt, ...);
int b2nsm_fatal(const char* newstate, const char* fmt, ...);
void b2nsm_logging(FILE* fp);
int b2nsm_wait(float timeout);

//added by Konno
int b2nsm_nodepid(const char* node);
int b2nsm_nodeid(const char* node);

#if defined(__dummy_open_bracket_to_cheat_emacs_auto_indent)
__dummy_open_bracket_to_cheat_emacs_auto_indent {
#endif
#if defined(__cplusplus)
}
#endif
