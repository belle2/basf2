/* ---------------------------------------------------------------------- *\

   ft3f.h

   Definitions for the ft3f registers and a struct

   Mikihiko Nakao, KEK IPNS

   20140409 first version

   ------------------------
   ft3f register definition
   ------------------------

   58--5f DUMPO
      [31:0] (32-bit) octet dump (total 32 octets)

   60 DUMPK
      [31:0] (32-bit) K-symbol flag dump (total 32 octets)

   65 DUMPI
      [31:0] (32-bit) debug info for dump

   66-6f DUMP
      [31:0] (32-bit) binary dump (total 32 octets)

\* ---------------------------------------------------------------------- */

static const int ft3f_revision = 1;

#define FT3F_STA      FTSWREG(0x20)  /* R: status */

#define FT3F_DUMPO0   FTSWREG(0x58)  /* R:  encoded octet dump */
#define FT3F_DUMPO1   FTSWREG(0x59)  /* R:  encoded octet dump */
#define FT3F_DUMPO2   FTSWREG(0x5a)  /* R:  encoded octet dump */
#define FT3F_DUMPO3   FTSWREG(0x5b)  /* R:  encoded octet dump */
#define FT3F_DUMPO4   FTSWREG(0x5c)  /* R:  encoded octet dump */
#define FT3F_DUMPO5   FTSWREG(0x5d)  /* R:  encoded octet dump */
#define FT3F_DUMPO6   FTSWREG(0x5e)  /* R:  encoded octet dump */
#define FT3F_DUMPO7   FTSWREG(0x5f)  /* R:  encoded octet dump */
#define FT3F_DUMPK    FTSWREG(0x60)  /* R:  encoded K-symbol flag dump */
#define FT3F_DUMPI    FTSWREG(0x65)  /* R:  debug info for dump */

#define FT3F_DUMP0    FTSWREG(0x66)  /* R:  raw input dump */
#define FT3F_DUMP1    FTSWREG(0x67)  /* R:  raw input dump */
#define FT3F_DUMP2    FTSWREG(0x68)  /* R:  raw input dump */
#define FT3F_DUMP3    FTSWREG(0x69)  /* R:  raw input dump */
#define FT3F_DUMP4    FTSWREG(0x6a)  /* R:  raw input dump */
#define FT3F_DUMP5    FTSWREG(0x6b)  /* R:  raw input dump */
#define FT3F_DUMP6    FTSWREG(0x6c)  /* R:  raw input dump */
#define FT3F_DUMP7    FTSWREG(0x6d)  /* R:  raw input dump */
#define FT3F_DUMP8    FTSWREG(0x6e)  /* R:  raw input dump */
#define FT3F_DUMP9    FTSWREG(0x6f)  /* R:  raw input dump */

struct ft3f {
  int32_t sta; /* FT3F_STA 20(R) */

  int32_t dumpo[8]; /* FT3F_DUMPO   58-5f(R)  dump octet */
  int32_t dumpk;    /* FT3F_DUMPK   60(R)     dump K-symbol flag */
  int32_t dumpi;    /* FT3F_DUMPI   65(R)     dump info */
  int32_t dump[10]; /* FT3F_DUMP    66-6f(R)  raw stream dump */
};

typedef struct ft3f ft3f_t;

/*
alias  buf_octet     : byte_t      is sta_reg(16#20#)(7 downto 0);
alias  buf_isk       : std_logic   is sta_reg(16#20#)(8);
alias  sta_comma     : std_logic   is sta_reg(16#20#)(9);
alias  sig_bit2      : std_logic2  is sta_reg(16#20#)(11 downto 10);
alias  cnt_bit2      : std_logic3  is sta_reg(16#20#)(14 downto 12);

alias  sig_runreset  : std_logic   is sta_reg(16#20#)(28);
alias  sig_trig      : std_logic   is sta_reg(16#20#)(29);
alias  sta_clkup     : std_logic   is sta_reg(16#20#)(30);
alias  sta_ttup      : std_logic   is sta_reg(16#20#)(31);

/*
-- constants
constant MAX_REG : integer := 16#6F#;
constant MIN_REG : integer := 16#10#;
constant XVERSION : long_t := conv_std_logic_vector(VERSION, 32);
constant INI_REG : long_vector (255 downto 0) := (
  16#10# => ID,
  16#17# => x"07952980", -- c1sec 127216000
  16#44# => x"80000000", -- fifoen
  16#4e# => x"23000f10", -- selaux
  16#34# => x"11223344",
  16#35# => x"55667788",
  16#36# => x"22446688",
  16#37# => x"11335577",
  16#38# => x"aaaa5555",
  16#39# => x"bbbb6666",
  others => x"00000000" );
constant CLR_REG : long_vector (255 downto 0) := (
  16#31# => x"FFFFFFFF",              -- set_odelay / clr_odelay
  others => x"00000000" );
constant RW_REG : std_logic_vector (255 downto 0) := (
  16#10# => '1', -- ID
  --16#12# => '1', -- UTIME
  --16#13# => '1', -- FREQ
  16#18# => '1', -- QUERY
  16#31# => '1', -- MANUAL

  16#4b# => '1', -- TRGDBG
  16#4e# => '1', -- SELAUX
  16#54# => '1', -- DBG
  others => '0' );

-- signals
signal set_reg : std_logic_vector (MAX_REG downto MIN_REG);
signal get_reg : std_logic_vector (MAX_REG downto MIN_REG);
signal buf_reg : long_vector (MAX_REG downto MIN_REG);
signal sta_reg : long_vector (MAX_REG downto MIN_REG) := (
  16#11# => XVERSION,
  others => x"00000000" );

signal sig_din  : long_t;
signal sig_dout : long_t;

--- registers: id
alias  sta_id     : std_logic16 is sta_reg(16#11#)(31 downto 16);

--- registers: timestamps (#120 to #130, #170 to #180)
--alias  set_utime     : std_logic   is set_reg(16#12#);
--alias  reg_utime     : long_t      is buf_reg(16#12#);
--alias  reg_clkfreq   : std_logic24 is buf_reg(16#13#)(23 downto 0);
alias  sta_utime     : long_t      is sta_reg(16#14#);
alias  sta_ctime     : std_logic27 is sta_reg(16#15#)(26 downto 0);

--- registers: FT-switch control (#180 to #190)
alias  reg_query     : std_logic   is buf_reg(16#18#)(31);
alias  sta_s3        : std_logic10 is sta_reg(16#19#)(25 downto 16);

--- registers: jitterspi
alias  sta_jpll      : std_logic   is sta_reg(16#1f#)(31);

--- registers: decode (#200 to #240)
alias  buf_octet     : byte_t      is sta_reg(16#20#)(7 downto 0);
alias  buf_isk       : std_logic   is sta_reg(16#20#)(8);
alias  sta_comma     : std_logic   is sta_reg(16#20#)(9);
alias  sig_bit2      : std_logic2  is sta_reg(16#20#)(11 downto 10);
alias  cnt_bit2      : std_logic3  is sta_reg(16#20#)(14 downto 12);

alias  sig_runreset  : std_logic   is sta_reg(16#20#)(28);
alias  sig_trig      : std_logic   is sta_reg(16#20#)(29);
alias  sta_clkup     : std_logic   is sta_reg(16#20#)(30);
alias  sta_ttup      : std_logic   is sta_reg(16#20#)(31);
-----  cnt_validclk  : std_logic28 is sta_reg(16#21#)(27 downto 0);
-----  cnt_validrevo : std_logic17 is sta_reg(16#22#)(16 downto 0);
-----  cnt_norevo    : std_logic17 is sta_reg(16#23#)(16 downto 0);

--- manual delay control (#300 to #310)
alias  cnt_delay     : std_logic12 is sta_reg(16#30#)(11 downto 0);
alias  reg_imanual   : std_logic   is buf_reg(16#31#)(7);
alias  clr_idelay    : std_logic   is buf_reg(16#31#)(2);
alias  set_idelay    : std_logic   is buf_reg(16#31#)(1);
alias  reg_slip      : std_logic   is buf_reg(16#31#)(0);

--- fifo
alias  sta_fifod0    : long_t      is sta_reg(16#32#);
alias  sta_fifod1    : long_t      is sta_reg(16#33#);
alias  sta_fifordy   : std_logic   is sta_reg(16#34#)(0);
alias  sta_fifofull  : std_logic   is sta_reg(16#34#)(1);
alias  sta_fifodbg   : std_logic18 is sta_reg(16#34#)(25 downto 8);
alias  sig_fifonext  : std_logic   is set_reg(16#34#);
-----  sta_fifodrd0  : long_t      is sta_reg(16#35#);
-----  sta_fifodrd1  : long_t      is sta_reg(16#36#);

alias  sta_dbg       : long_t      is sta_reg(16#47#);
-----  sta_dbg       : byte_t      is sta_reg(16#47#)(7  downto 0);
alias  sta_dbg2      : long_t      is sta_reg(16#48#);

--- trgtag
alias  sta_trgtag    : long_t      is sta_reg(16#49#);
alias  sta_trgtyp    : std_logic4  is sta_reg(16#4a#)(31 downto 28);
alias  reg_trgdbg    : std_logic11 is sta_reg(16#4b#)(10 downto 0);
alias  sta_timtrig   : long_t      is sta_reg(16#4c#);
alias  sta_timttag   : long_t      is sta_reg(16#4d#);

alias  reg_selaux1   : std_logic5  is buf_reg(16#4e#)(4  downto 0);
alias  reg_selaux2   : std_logic5  is buf_reg(16#4e#)(12 downto 8);
alias  reg_regdbg    : byte_t      is buf_reg(16#4e#)(31 downto 24);

alias  cnt_rstin     : byte_t      is sta_reg(16#51#)(7  downto 0);
alias  sta_urstin    : long_t      is sta_reg(16#52#);
alias  sta_crstin    : std_logic27 is sta_reg(16#53#)(26 downto 0);

alias  reg_handbusy  : std_logic is buf_reg(16#54#)(0);
alias  reg_handerr   : std_logic is buf_reg(16#54#)(1);

--- registers: dump (#580 to #6f0, but #610 and #620 are still free)
alias  sta_10b  : std_logic10 is sta_reg(16#61#)(9 downto 0);

alias  cnt_error     : long_t      is sta_reg(16#63#);
alias  cnt_octet     : std_logic5  is sta_reg(16#64#)(4 downto 0);

alias  set_dump      : std_logic   is set_reg(16#65#);
alias  buf_dumpi     : long_t      is sta_reg(16#65#);
alias  buf_dumpk     : long_t      is sta_reg(16#60#);
alias  buf_dump8 : long_vector (7 downto 0) is sta_reg(16#5f# downto 16#58#);
alias  buf_dump2 : long_vector (9 downto 0) is sta_reg(16#6f# downto 16#66#);

*/
