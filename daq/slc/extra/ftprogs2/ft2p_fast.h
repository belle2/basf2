
static const int ft2p_fast_revision = 1;

struct ft2p_fast {
  /* process variables */
  uint8_t isrunning; // 0: ready, 1: running, 2: paused
  uint8_t sigpipe;
  uint8_t verbose;
  uint8_t ebisbusy;

  int32_t evt_number;
  int32_t evt_total;
  /* ft2p registers */
  uint32_t utime;      /* f14 */
  uint32_t ctime;      /* f15 */
  uint32_t errport;    /* f25 */
  uint32_t tincnt;     /* f2a */
  uint32_t toutcnt;    /* f2b */
  uint32_t tlast;      /* f2c */
  uint32_t stafifo;    /* f2d */
  uint32_t obusy;      /* f30 */
  uint32_t obsyin;     /* f31 */
  uint32_t oerrin;     /* f32 */
  uint32_t olinkdn;    /* f33 */
  uint32_t oalive;     /* f34 */
  uint32_t olinkup;    /* f35 */
  uint32_t b2ldn;      /* f36 */
  uint32_t plldn;      /* f37 */
  uint32_t odata[12];  /* f40_4b */
  uint32_t odatb[12];  /* f54_5f */
  uint32_t odatc[12];  /* f68_73 */
  uint32_t errsrc;     /* f9e */
  uint32_t atimec;     /* fa0 */
  uint32_t atrigc;     /* fa1 */
  uint32_t abusyc;     /* fa2 */
  uint32_t tbusyc;     /* fa3 */
  uint32_t cbusyc;     /* fa4 */
  uint32_t pbusyc;     /* fa5 */
  uint32_t fbusyc;     /* fa6 */
  uint32_t rbusyc;     /* fa7 */
  uint32_t obusyc[12]; /* fb0_bb */
};
