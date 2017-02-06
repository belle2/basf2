/* ---------------------------------------------------------------------- *\

   ftproglib.h - commonly used ftprogs functions

   Mikihiko Nakao, KEK IPNS

   2016.0413.0955 new

\* ---------------------------------------------------------------------- */

#include "ftsw.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
void utimeft(ftsw_t* ftsw, const char* PROGRAM);
uint32_t resetft(ftsw_t* ftsw,
                 int doreset,
                 int dofifo,
                 int dowait,
                 int dobor,
                 int notagerr,
                 int trgtype,
                 const char* PROGRAM);
#ifdef __cplusplus
}
#endif /* __cplusplus */
