#ifndef _statft_h

#ifndef D
#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __to_fake_emacs
};
#endif

//void statft(ftsw_t *ftsw, int ftswid, char* ss)
void statft_jitter(ftsw_t* ftsw, int reg_jctl, int reg_jreg, char* ss);
void statft_headline(int ftswid, int fpgaid, int fpgaver, struct timeval* tvp, char* ss);
const char* localtimestr(int utim);
const char* dumtrgstr(int rate);

char ftstat_err[100];

#ifdef __to_cheat_emacs
extern "C" {
#endif
#ifdef __cplusplus
};
#endif

#endif

