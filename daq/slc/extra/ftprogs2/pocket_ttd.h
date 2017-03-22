/*
  20131227 rev.7  merging pocket_ttd and statft
  20131228 rev.8  first 4 bytes to be only in ttd_fast
  20140103 rev.9  stdint version, restart number
  20160330 rev.10 update for pocket_ttd-desy
 */

static const int pocket_ttd_revision = 10;

struct pocket_ttd {
  /* tmp */
  int16_t b2ebport;
  int16_t b2ebsock;
  int32_t b2ebip;
  int32_t myip;
  int16_t exp_number;
  int16_t run_number;
  int16_t restart_number;
  int16_t run_count;
  int32_t pause_evt;
  /* (fast) int32_t evt_number; */
  /* (fast) int32_t evt_total; */
  /* -- FTSW registers -- */
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
  uint32_t jrst;       /* w1e */
  uint32_t jsta;       /* s1f */
  uint32_t reset;      /* w20 */
  uint32_t rstutim;    /* s21 */
  uint32_t rstctim;    /* s22 */
  uint32_t errutim;    /* s23 */
  uint32_t errctim;    /* s24 */
  uint32_t seltrg;     /* w28 */
  uint32_t tlimit;     /* w29 */
  uint32_t enstat;     /* s30 */
  uint32_t revopos;    /* w31 */
  uint32_t revocand;   /* s32 */
  uint32_t badrevo;    /* s33 */
  uint32_t maxtrig;    /* w50 */
  uint32_t xbcnt[4];   /* s6c_6f */
  uint32_t omanual;    /* w7c */
  uint32_t xmanual;    /* w7d */
  uint32_t dbg;        /* s7e */
  uint32_t dbgb;       /* s7f */
  uint32_t crc;        /* s9d */
  uint32_t tdelay;     /* w9f */
};
