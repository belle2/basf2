
static const int pocket_ttd_fast_revision = 5;

struct pocket_ttd_fast {
  uint8_t isrunning; // 0: ready, 1: running, 2: paused
  uint8_t sigpipe;
  uint8_t verbose;
  uint8_t ebisbusy;
  /* tmp */
  int32_t evt_number;
  int32_t evt_total;
  /* FTSW registers */
  uint32_t utime;      /* f14 */
  uint32_t ctime;      /* f15 */
  uint32_t errport;    /* f25 */
  uint32_t tincnt;     /* f2a */
  uint32_t toutcnt;    /* f2b */
  uint32_t tlast;      /* f2c */
  uint32_t stafifo;    /* f2d */
  uint32_t stat;       /* f38 */
  uint32_t linkup;     /* f39 */
  uint32_t error;      /* f3a */
  uint32_t linkerr;    /* f3b */
  uint32_t odata[8];   /* f3c_43 */
  uint32_t odatb[8];   /* f44_4b */
  uint32_t xdata[4];   /* f4c_4f */
  uint32_t tlu;        /* s52 */
  uint32_t tluutim;    /* s53 */
  uint32_t tluctim;    /* s54 */
  uint32_t udead;      /* s5a */
  uint32_t cdead;      /* s5b */
  uint32_t pdead;      /* s5c */
  uint32_t edead;      /* s5d */
  uint32_t fdead;      /* s5e */
  uint32_t rdead;      /* s5f */
  uint32_t odead[8];   /* s60_67 */
  uint32_t xdead[4];   /* s68_6b */
  uint32_t odatc[8];   /* f70_77 */
  uint32_t xdatb[4];   /* f78_7b */
  uint32_t errsrc;     /* f9e */
};
