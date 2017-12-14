
/* common statft macros */

#ifndef P
#define P printf
#define PP printf
#endif

#ifndef D
#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")
#endif

#ifndef ASSERT
#define WARN(msg,cond)            \
  do {if(cond){PP("\n"); P("WARNING: %s %s!\n\n", msg, #cond); } } while(0)
#define ASSERT(msg,cond)          \
  do {if(cond){ PP("\n"); \
      P("%s: %s %s!\n", doassert?"ABORT":"ERROR",msg, #cond); \
      if(doassert) return;} } while(0)
#endif

#ifndef STATFT_MAIN

#include "ftstat.h"

/* common statft global variables */

#if !defined(TTMASTER) && !defined(POCKET_TTD)
extern int VERSION;
extern char* PROGRAM;
extern int verbose;
extern int jitter;
extern int doassert;
extern int interval;
extern int showjitter;
extern int showdump;
extern int showcolor;
#endif

/* common statft functions */

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __to_fake_emacs
};
#endif
void statft_assert(struct timeval* tvp, int id,
                   int conf, int fpgaver, int utime, int jsta);
void statft_headline(int ftswid, int fpgaid, int fpgaver, struct timeval* tvp);
void statft_jitter(ftsw_t* ftsw, int reg_jctl, int reg_jreg);
void statft_dump(int dumpk, int dumpi, int dumpo[], int dump2[]);
void setcolor(int color, const char* text);
const char* localtimestr(int utim);
const char* tdiffstr(int tdiff);

void regs2u041(ftsw_t* ftsw, struct timeval* tvp, fast_t* f, slow_t* s);
void summary2u041(struct timeval* tvp, fast_t* f, slow_t* s);
void verbose2u041(struct timeval* tvp, fast_t* f, slow_t* s);

#ifdef __to_cheat_emacs
extern "C" {
#endif
#ifdef __cplusplus
};
#endif

#endif /* STATFT_MAIN */

