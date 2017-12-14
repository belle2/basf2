#ifndef _ca2nsm_callback_h
#define _ca2nsm_callback_h

#ifdef __cplusplus
extern "C" {
#endif

void init_ca2nsm(const char* confname);
void add_ca2nsm(const char* pvname);
void add_PVs();

#ifdef __cplusplus
}
#endif

#endif
