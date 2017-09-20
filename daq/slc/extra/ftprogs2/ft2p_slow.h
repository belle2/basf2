
static const int ft2p_slow_revision = 1;

struct ft2p_slow {
  /* process variables */
  int16_t b2ebport;
  int16_t b2ebsock;
  int32_t b2ebip;
  int32_t myip;
  int16_t exp_number;
  int16_t run_number;
  int16_t restart_number;
  int16_t run_count;
  int32_t pause_evt;


  /* ft2p registers */
  uint32_t ftswid;     /* w00 */
  uint32_t cpldver;    /* s01 */
  uint32_t conf;       /* w02 */
  uint32_t clksel;     /* w04 */
  uint32_t fpgaid;     /* w10 */
  uint32_t fpgaver;    /* s11 */
  uint32_t setutim;    /* w12 */
  uint32_t clkfreq;    /* w13 */
  uint32_t exprun;     /* w16 */
  uint32_t omask;      /* w17 */
  uint32_t addr;       /* w18 */
  uint32_t cmd;        /* w19 */
  uint32_t jtag;       /* w1a */
  uint32_t tdo;        /* s1b */
  uint32_t jctl;       /* w1c */
  uint32_t jreg;       /* s1d */
  uint32_t jpd;        /* w1e */
  uint32_t jpll;       /* s1f */
  uint32_t reset;      /* w20 */
  uint32_t rstutim;    /* s21 */
  uint32_t rstctim;    /* s22 */
  uint32_t errutim;    /* s23 */
  uint32_t errctim;    /* s24 */
  uint32_t seltrg;     /* w28 */
  uint32_t tlimit;     /* w29 */
  uint32_t omanual;    /* w7c */
  uint32_t clrdelay;   /* w7d */
  uint32_t incdelay;   /* w7e */
  uint32_t latency;    /* w7f */
  uint32_t lckfreq;    /* s80 */
  uint32_t revopos;    /* w81 */
  uint32_t revocand;   /* s82 */
  uint32_t badrevo;    /* s83 */
  uint32_t oackq;      /* s84 */
  uint32_t dbg[3];     /* s85_87 */
  uint32_t dumpo[8];   /* s88_8f */
  uint32_t dumpb[10];  /* s90_99 */
  uint32_t dumpk;      /* s9a */
  uint32_t dumpi;      /* s9b */
  uint32_t disp;       /* w9c */
  uint32_t tdelay;     /* w9f */
  uint32_t reg3s;      /* sa8 */
};
