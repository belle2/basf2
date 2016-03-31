/*
  20131227 rev.1  merging pocket_ttd and statft
 */

static const int pocket_ttd_dump_revision = 1;

struct pocket_ttd_dump {
  uint32_t dumpo[8];   /* s80_87 */
  uint32_t dumpk;      /* s88 */
  uint32_t dumpi;      /* s89 */
  uint32_t oackq;      /* s8a */
  uint32_t dumpb[10];  /* s90_99 */
  uint32_t dumpc;      /* w9c */
};
