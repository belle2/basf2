-------------------------------------------------------------------------------
--
-- File Name : vtd_0.02.vhd
--    Author : Y.Iwasaki
--   History : 00 : 2014/04/21 : First version
--
-------------------------------------------------------------------------------

library IEEE;
  use   IEEE.std_logic_1164.all;
  use   IEEE.std_logic_unsigned.all;
--use   IEEE.std_logic_arith.all;
  use   IEEE.NUMERIC_STD.all;
  use   STD.TEXTIO.all;
library work;
--use   work.L1_defs.all; 
library UNISIM;
  use   UNISIM.VCOMPONENTS.ALL;

entity FTD is
generic (
  I_WIDTH : integer := 76;
  O_WIDTH     : integer := 13
--  constant FTD_VERSION : std_logic_vector(31 downto 0)   := X"47444C20";  -- GDL
--  constant OFFSET_SCL1 : integer := 24;
--  constant OFFSET_SCL2 : integer := 24
);
port (

    -- Reset
    VERSION    : out std_logic_vector(9 downto 0);

    CLK    : in  std_logic;
    SUBT   : in  std_logic_vector(I_WIDTH - 1 downto 0);
    FTDOUT : out std_logic_vector(O_WIDTH - 1 downto 0)

);

end FTD;

-------------------------------------------------------------------------------

architecture RTL of FTD is

  constant ftd_version : std_logic_vector(9 downto 0) := "0000000011"; -- 0.03

  --...................................
  --   Other signals
  --...................................

  signal sig_ftdout : std_logic_vector(O_WIDTH - 1 downto 0);

  --...................................
  --   Inputs -- keep this comment here
  --...................................

  --...CDC...

  signal n_t3_full  : std_logic_vector(2 downto 0);
  signal n_t3_short : std_logic_vector(2 downto 0);
  signal n_t2_full  : std_logic_vector(2 downto 0);
  signal n_t2_short : std_logic_vector(2 downto 0);
  signal cdc_bb     : std_logic := '0';
  signal cdc_open45 : std_logic := '0';
  signal cdc_active : std_logic := '0';
  signal cdc_timing : std_logic_vector(2 downto 0);

  --...ECL...

  signal e_high         : std_logic := '0';
  signal e_low          : std_logic := '0';
  signal e_lum          : std_logic := '0';
  signal ecl_bha        : std_logic := '0';
  signal bha_type       : std_logic_vector(10 downto 0);
  signal n_clus         : std_logic_vector(3 downto 0);
  signal bg_ecl_veto    : std_logic := '0';
  signal ecl_timing_fwd : std_logic := '0';
  signal ecl_timing_bwd : std_logic := '0';
  signal ecl_timing_brl : std_logic := '0';
  signal ecl_active     : std_logic := '0';
  signal ecl_timing     : std_logic_vector(2 downto 0);

  --...TOP...

  signal n_top      : std_logic_vector(2 downto 0);
  signal top_bb     : std_logic := '0';
  signal top_active : std_logic := '0';
  signal top_timing : std_logic_vector(3 downto 0);

  --...KLM...

  signal n_klm : std_logic_vector(2 downto 0);

  --..KEKB...

  signal revo     : std_logic := '0';
  signal her_kick : std_logic := '0';
  signal ler_kick : std_logic := '0';

  --...NIM input
  signal nimin0      : std_logic := '0';
  signal nimin1      : std_logic := '0';
  signal nimin2      : std_logic := '0';
  signal crt_out     : std_logic := '0';

  --...GDL Internal...

  signal bha_delay   : std_logic := '0';
  signal pseude_rand : std_logic := '0';
  signal veto        : std_logic := '0';
  signal n_t3        : std_logic_vector(2 downto 0);
  signal n_t2        : std_logic_vector(2 downto 0);

  --....................................
  --   Outputs -- keep this comment here
  --....................................

  signal zzx        : std_logic := '0'; -- 0
  signal ffs        : std_logic := '0'; -- 1
  signal zx         : std_logic := '0'; -- 2
  signal fs         : std_logic := '0'; -- 3
  
  signal hie        : std_logic := '0'; -- 4
  signal c4         : std_logic := '0'; -- 5

  signal bhabha     : std_logic := '0'; -- 6
  signal bhabha_trk : std_logic := '0'; -- 7

  signal gg         : std_logic := '0'; -- 8

  signal mu_pair    : std_logic := '0'; -- 9

  signal revolution : std_logic := '0'; -- 10
  signal random     : std_logic := '0'; -- 11
  signal bg         : std_logic := '0'; -- 12

  signal nim0      : std_logic := '0';
  signal nim1      : std_logic := '0';
  signal nim2      : std_logic := '0';
  signal crtout    : std_logic := '0';

begin

  VERSION <= ftd_version;
  FTDOUT  <= sig_ftdout; 
          
  --...................................
  --   Inputs
  --...................................

  n_t3_full(0)   <= SUBT(0);
  n_t3_full(1)   <= SUBT(1);
  n_t3_full(2)   <= SUBT(2);
  n_t3_short(0)  <= SUBT(3);
  n_t3_short(1)  <= SUBT(4);
  n_t3_short(2)  <= SUBT(5);
  n_t2_full(0)   <= SUBT(6);
  n_t2_full(1)   <= SUBT(7);
  n_t2_full(2)   <= SUBT(8);
  n_t2_short(0)  <= SUBT(9);
  n_t2_short(1)  <= SUBT(10);
  n_t2_short(2)  <= SUBT(11);
  cdc_bb         <= SUBT(12);
  cdc_open45     <= SUBT(13);
  cdc_active     <= SUBT(14);
  cdc_timing(0)  <= SUBT(15);
  cdc_timing(1)  <= SUBT(16);
  cdc_timing(2)  <= SUBT(17);

  e_high         <= SUBT(18);
  e_low          <= SUBT(19);
  e_lum          <= SUBT(20);
  ecl_bha        <= SUBT(21);
  bha_type(0)    <= SUBT(22);
  bha_type(1)    <= SUBT(23);
  bha_type(2)    <= SUBT(24);
  bha_type(3)    <= SUBT(25);
  bha_type(4)    <= SUBT(26);
  bha_type(5)    <= SUBT(27);
  bha_type(6)    <= SUBT(28);
  bha_type(7)    <= SUBT(29);
  bha_type(8)    <= SUBT(30);
  bha_type(9)    <= SUBT(31);
  bha_type(10)   <= SUBT(32);
  n_clus(0)      <= SUBT(33);
  n_clus(1)      <= SUBT(34);
  n_clus(2)      <= SUBT(35);
  n_clus(3)      <= SUBT(36);
  bg_ecl_veto    <= SUBT(37);
  ecl_timing_fwd <= SUBT(38);
  ecl_timing_bwd <= SUBT(39);
  ecl_timing_brl <= SUBT(40);
  ecl_active     <= SUBT(41);
  ecl_timing(0)  <= SUBT(42);
  ecl_timing(1)  <= SUBT(43);
  ecl_timing(2)  <= SUBT(44);

  n_top(0)       <= SUBT(45);
  n_top(1)       <= SUBT(46);
  n_top(2)       <= SUBT(47);
  top_bb         <= SUBT(48);
  top_active     <= SUBT(49);
  top_timing(0)  <= SUBT(50);
  top_timing(1)  <= SUBT(51);
  top_timing(2)  <= SUBT(52);
  top_timing(3)  <= SUBT(53);
  
  n_klm(0)       <= SUBT(54);
  n_klm(1)       <= SUBT(55);
  n_klm(2)       <= SUBT(56);
  
  revo           <= SUBT(57);
  her_kick       <= SUBT(58);
  ler_kick       <= SUBT(59);
  
  bha_delay      <= SUBT(60);
  pseude_rand    <= SUBT(61);
  veto           <= SUBT(62);
  
  nimin0         <= SUBT(63);
  nimin1         <= SUBT(64);
  nimin2         <= SUBT(65);
  crt_out        <= SUBT(66);

  --...GDL internal... should be moved before FTD
  n_t3           <= n_t3_full + n_t3_short;
  n_t2           <= n_t2_full + n_t2_short;
  
  --.......................................
  --   FTD logics
  --.......................................

  zzx        <= '1' when (n_t3_full>1) and (n_t3_short>0) and cdc_open45='1'
                and (veto='0') else '0';  --  0
  ffs        <= '1' when (n_t2_full>1) and (n_t2_short>0) and cdc_open45='1'
                and (veto='0') else '0';  --  1
  zx         <= '1' when (n_t3_full>0) and (n_t3_short>0) and cdc_open45='1' and (ecl_bha='0')
                and (veto='0') else '0';  --  2
  fs         <= '1' when (n_t2_full>0) and (n_t2_short>0) and cdc_open45='1' and (ecl_bha='0')
                and (veto='0') else '0';  --  3

  hie        <= '1' when e_high='1' and (ecl_bha='0')
                and (veto='0') else '0';  --  4
  c4         <= '1' when (n_clus>3)
                and (veto='0') else '0';  --  5
  bhabha     <= '1' when ecl_bha='1'
                and (veto='0') else '0';  --  6
  bhabha_trk <= '1' when ecl_bha='1' and (n_t2>1) and cdc_bb='1'
                and (veto='0') else '0';  --  7
  gg         <= '1' when e_high='1' and (n_t2=0) and (n_t3=0)
                and (veto='0') else '0';  --  8

  mu_pair    <= '1' when (n_klm>0) and (n_t2>1)
                and (veto='0') else '0';  --  9

  revolution <= revo;
  random     <= pseude_rand;
  bg         <= bha_delay;

  nim0       <= nimin0;
  nim1       <= nimin1;
  nim2       <= nimin2;
  crtout     <= crt_out;

  process (clk)
  begin
    if (clk'event and clk = '1') then
      sig_ftdout(16) <= crtout;
      sig_ftdout(15) <= nim2;
      sig_ftdout(14) <= nim1;
      sig_ftdout(13) <= nim0;
      sig_ftdout(12) <= bg;
      sig_ftdout(11) <= random;
      sig_ftdout(10) <= revolution;
      sig_ftdout(9) <= mu_pair;
      sig_ftdout(8) <= gg;
      sig_ftdout(7) <= bhabha_trk;
      sig_ftdout(6) <= bhabha;
      sig_ftdout(5) <= c4;
      sig_ftdout(4) <= hie;
      sig_ftdout(3) <= fs;
      sig_ftdout(2) <= zx;
      sig_ftdout(1) <= ffs;
      sig_ftdout(0) <= zzx;
    end if;
  end process;

end RTL;
