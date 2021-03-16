-------------------------------------------------------------------------------
--
-- File Name : vtd_0.02.vhd
--    Author : Y.Iwasaki
--   History : 00 : 2014/04/21 : First version
--

-------------------------------------------------------------------------------
--
-- For vhdl-to-alg-file converter script,
--
--  o Logic must be in the form of "A and B", not in "A or B".
--
--     OK: (A or B or C) and (D or E)
--     not OK: (A and B) or (C and D)
--     not OK: (A or B) or (C and D)
--
--     "or" will be performed for PSNM output!
--
--  o Logic using "downto" (bha_type(1 downto 0) /= "00") not allowed 
--
--  o input bits are extracted from '<= SUBT(' lines. 
--    Do not change signal name, SUBT
--
--  o output bits are extracted from 'sig_ftdout(' lines. 
--    Do not change signal name, sig_ftdout.
--
--  o algorithms are extracted from "xxx <= '1' when" lines,
--     where xxx is examined if it's included in output bits.
--     Remove unnecessary parentheses as much as possible.

library IEEE;
  use   IEEE.std_logic_1164.all;
  use   IEEE.std_logic_unsigned.all;
--use   IEEE.std_logic_arith.all;
  use   IEEE.NUMERIC_STD.all;
  use   STD.TEXTIO.all;
library work;
  use   work.defs_gdl.all; 
library UNISIM;
  use   UNISIM.VCOMPONENTS.ALL;

entity FTD is
generic (
  I_WIDTH : integer := 160;
  O_WIDTH : integer := 160
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

  constant ftd_version : std_logic_vector(9 downto 0) := "0000100001"; -- 0.33

  --...................................
  --   Other signals
  --...................................

  signal sig_ftdout : std_logic_vector(O_WIDTH - 1 downto 0);

  --...................................
  --   Inputs -- keep this comment here
  --...................................

  --...CDC... 0-21

  signal t3   : std_logic_vector(3 downto 0);--0
  signal ty   : std_logic_vector(3 downto 0);--4-7
  signal t2   : std_logic_vector(3 downto 0);--8-11
  signal ts   : std_logic_vector(3 downto 0);--12-15
  signal ta   : std_logic_vector(3 downto 0);--16-19

  signal cdc_open90 : std_logic := '0';--20
  signal cdc_active : std_logic := '0';
  signal cdc_b2b3   : std_logic := '0';
  signal cdc_b2b5   : std_logic := '0';
  signal cdc_b2b7   : std_logic := '0';
  signal cdc_b2b9   : std_logic := '0';--25

  --...ECL...

  signal ehigh          : std_logic := '0';--26
  signal elow           : std_logic := '0';
  signal elum           : std_logic := '0';
  signal ecl_bha        : std_logic := '0';
  signal ecl_3dbha      : std_logic := '0';
  signal bha_veto       : std_logic := '0';
  signal bha_type       : std_logic_vector(1 downto 0);--32-33
  signal ti             : std_logic := '0';
  signal ecleklm        : std_logic := '0';
  signal ieklm          : std_logic := '0';
  signal fwd_s          : std_logic := '0';--37
  signal bwd_s          : std_logic := '0';--38
  signal bha_intrk      : std_logic := '0';--39
  signal bha_theta      : std_logic_vector(1 downto 0);--40-41
  signal tx             : std_logic_vector(3 downto 0);--42-45
  signal clst           : std_logic_vector(3 downto 0);--46-49
  signal i2fo           : std_logic := '0';
  signal secl           : std_logic := '0';
  signal iecl           : std_logic := '0';
  signal ecl_active     : std_logic := '0';
  signal ecl_timing_fwd : std_logic := '0';
  signal ecl_timing_brl : std_logic := '0';
  signal ecl_timing_bwd : std_logic := '0';
  signal ecl_phys       : std_logic := '0';--57
  signal ecl_oflo       : std_logic := '0';--58
  signal ecl_lml        : std_logic_vector(10 downto 0);--59-69
  signal ecl_lml_12     : std_logic := '0';--70
  signal ecl_lml_13     : std_logic := '0';
  signal ecl_mumu       : std_logic := '0';
  signal ecl_bhapur     : std_logic := '0';
  signal ecl_bst        : std_logic := '0';--74

  --...TOP...

  signal top      : std_logic_vector(2 downto 0);-- 75-77
  signal top_bb     : std_logic := '0';
  signal top_active : std_logic := '0';--79

  --...KLM...  

  signal klm_hit : std_logic := '0';--80
  signal klm : std_logic_vector(2 downto 0);
  signal klmb2b : std_logic := '0';
  signal eklm_hit : std_logic := '0';
  signal eklm : std_logic_vector(2 downto 0);
  signal eklmb2b : std_logic := '0';--89


  --..KEKB...

  signal revo     : std_logic := '0';--90
  signal her_kick : std_logic := '0';--91
  signal ler_kick : std_logic := '0';--92

  --...GDL Internal...

  signal bha_delay  : std_logic := '0'; --93
  signal pseud_rand : std_logic := '0'; --
  signal plsin      : std_logic := '0'; --
  signal poissonin  : std_logic := '0'; --
  signal veto       : std_logic := '0'; --
  signal injv       : std_logic := '0'; --98

  --...GRL...

  signal samhem       : std_logic := '0';--99
  signal opohem       : std_logic := '0';
  signal cdcecl       : std_logic_vector(3 downto 0);--101,104
  signal n1gev       : std_logic_vector(3 downto 0);--105
  signal n2gev       : std_logic_vector(3 downto 0);--109
  signal c2gev       : std_logic_vector(3 downto 0);--113
  signal cdcklm       : std_logic_vector(1 downto 0);--117
  signal seklm        : std_logic_vector(1 downto 0);--119,120
  signal cdctop       : std_logic_vector(3 downto 0);--121,124
  signal d3           : std_logic := '0';--125
  signal d5           : std_logic := '0';
  signal d7           : std_logic := '0';
  signal d9           : std_logic := '0';
  signal p3           : std_logic := '0';
  signal p5           : std_logic := '0';
  signal p7           : std_logic := '0';
  signal p9           : std_logic := '0';
  signal track        : std_logic := '0';
  signal trkflt       : std_logic := '0';--134

  signal itsfb2b : std_logic := '0';--135
  signal f2f30 : std_logic := '0';
  signal s2f30 : std_logic := '0';
  signal s2s30 : std_logic := '0';

  signal s2s3     : std_logic := '0'; --139
  signal s2s5     : std_logic := '0'; --
  signal s2so     : std_logic := '0'; --
  signal s2f3     : std_logic := '0'; --
  signal s2f5     : std_logic := '0'; --
  signal s2fo     : std_logic := '0'; --
  signal fwdsb    : std_logic := '0'; --
  signal bwdsb    : std_logic := '0'; --
  signal fwdnb    : std_logic := '0'; --
  signal bwdnb    : std_logic := '0'; --
  signal brlfb1   : std_logic := '0'; --
  signal brlfb2   : std_logic := '0'; --
  signal brlnb1   : std_logic := '0'; --
  signal brlnb2   : std_logic := '0'; --
  signal trkbha1  : std_logic := '0'; --
  signal trkbha2  : std_logic := '0'; --
  signal grlgg1   : std_logic := '0'; --
  signal grlgg2   : std_logic := '0'; --156

  --...NIM input
  signal nimin0 : std_logic := '0';--157
  signal nimin1 : std_logic := '0';--158

  --....................................
  --   Outputs -- keep this comment here
  --....................................

  signal fff        : std_logic := '0'; --0
  signal ffs        : std_logic := '0'; --
  signal fss        : std_logic := '0'; --
  signal sss        : std_logic := '0'; --
  signal ffz        : std_logic := '0'; --
  signal fzz        : std_logic := '0'; --
  signal zzz        : std_logic := '0'; --
  signal ffy        : std_logic := '0'; --
  signal fyy        : std_logic := '0'; --
  signal yyy        : std_logic := '0'; --
  signal ff         : std_logic := '0'; -- 
  signal fs         : std_logic := '0'; -- 
  signal ss         : std_logic := '0'; -- 
  signal fz         : std_logic := '0'; -- 
  signal zz         : std_logic := '0'; -- 
  signal fy         : std_logic := '0'; -- 
  signal yy         : std_logic := '0'; -- 
  signal ffo        : std_logic := '0'; -- 
  signal fso        : std_logic := '0'; -- 
  signal sso        : std_logic := '0'; -- 
  signal fzo        : std_logic := '0'; -- 
  signal fyo        : std_logic := '0'; -- 
  signal ffb        : std_logic := '0'; -- 
  signal fsb        : std_logic := '0'; -- 
  signal ssb        : std_logic := '0'; -- 
  signal fzb        : std_logic := '0'; -- 
  signal fyb        : std_logic := '0'; -- 
  signal syo        : std_logic := '0'; -- 
  signal syb        : std_logic := '0'; -- 
  signal ff30       : std_logic := '0'; --
  signal fs30       : std_logic := '0'; --
  signal ss30       : std_logic := '0'; --
  signal fy30       : std_logic := '0'; --
  signal aaa        : std_logic := '0'; --
  signal aaao       : std_logic := '0'; --
  signal aao        : std_logic := '0'; --
  signal aab        : std_logic := '0'; --
  signal aa         : std_logic := '0'; --
  signal hie        : std_logic := '0'; --38 
  signal lowe       : std_logic := '0'; --
  signal lume       : std_logic := '0'; --
  signal hade       : std_logic := '0'; --
  signal c2         : std_logic := '0'; --
  signal c3         : std_logic := '0'; --
  signal c4         : std_logic := '0'; --
  signal c5         : std_logic := '0'; --
  signal bha3d      : std_logic := '0'; -- 
  signal bhabha     : std_logic := '0'; --
  signal bhapur     : std_logic := '0'; --
  signal bha_id0    : std_logic := '0'; --
  signal bha_id1    : std_logic := '0'; --
  signal bha_id2    : std_logic := '0'; --
  signal bha_id3    : std_logic := '0'; --
  signal eclmumu    : std_logic := '0'; --
  signal ecloflo    : std_logic := '0'; --
  signal eclbst     : std_logic := '0'; --
  signal g_high     : std_logic := '0'; --
  signal g_c1       : std_logic := '0'; --
  signal gg         : std_logic := '0'; --
  signal eed        : std_logic := '0'; --59
  signal fed        : std_logic := '0'; -- 
  signal fp         : std_logic := '0'; -- 
  signal sp         : std_logic := '0'; -- 
  signal zp         : std_logic := '0'; -- 
  signal yp         : std_logic := '0'; -- 
  signal d_5        : std_logic := '0'; --
  signal shem       : std_logic := '0'; --
  signal ohem       : std_logic := '0'; --
  signal toptiming  : std_logic := '0'; --68 
  signal ecltiming  : std_logic := '0'; -- 
  signal cdctiming  : std_logic := '0'; -- 
  signal cdcbb      : std_logic := '0'; --71
  signal mu_pair    : std_logic := '0'; --
  signal mu_b2b     : std_logic := '0'; --
  signal klmhit     : std_logic := '0'; --
  signal mu_epair   : std_logic := '0'; --
  signal mu_eb2b    : std_logic := '0'; --
  signal eklmhit    : std_logic := '0'; --
  signal revolution : std_logic := '0'; --78
  signal random     : std_logic := '0'; --
  signal bg         : std_logic := '0'; --
  signal pls        : std_logic := '0'; --
  signal poisson    : std_logic := '0'; --
  signal vetout     : std_logic := '0'; --
  signal f          : std_logic := '0'; --84
  signal s          : std_logic := '0'; --
  signal z          : std_logic := '0'; --
  signal y          : std_logic := '0'; --
  signal a          : std_logic := '0'; --
  signal n1gev1     : std_logic := '0'; --89
  signal n1gev2     : std_logic := '0'; --
  signal n1gev3     : std_logic := '0'; --
  signal n1gev4     : std_logic := '0'; --
  signal n2gev1     : std_logic := '0'; --
  signal n2gev2     : std_logic := '0'; --
  signal n2gev3     : std_logic := '0'; --
  signal n2gev4     : std_logic := '0'; --
  signal c2gev1     : std_logic := '0'; --
  signal c2gev2     : std_logic := '0'; --
  signal c2gev3     : std_logic := '0'; --
  signal c2gev4     : std_logic := '0'; --
  signal cdcecl1    : std_logic := '0'; --
  signal cdcecl2    : std_logic := '0'; --
  signal cdcecl3    : std_logic := '0'; --
  signal cdcecl4    : std_logic := '0'; --
  signal cdcklm1    : std_logic := '0'; --
  signal cdcklm2    : std_logic := '0'; --
  signal seklm1     : std_logic := '0'; --
  signal seklm2     : std_logic := '0'; --
  signal cdctop1    : std_logic := '0'; --109
  signal cdctop2    : std_logic := '0'; --
  signal cdctop3    : std_logic := '0'; --
  signal cdctop4    : std_logic := '0'; --
  signal c1hie      : std_logic := '0'; --
  signal c1lume     : std_logic := '0'; --
  signal n1hie      : std_logic := '0'; --
  signal n1lume     : std_logic := '0'; --
  signal c3hie      : std_logic := '0'; --
  signal c3lume     : std_logic := '0'; --
  signal n3hie      : std_logic := '0'; --119
  signal n3lume     : std_logic := '0'; --
  signal c2hie      : std_logic := '0'; --
  signal lml0       : std_logic := '0'; --122
  signal lml1       : std_logic := '0'; --
  signal lml2       : std_logic := '0'; --
  signal lml3       : std_logic := '0'; --
  signal lml4       : std_logic := '0'; --
  signal lml5       : std_logic := '0'; --
  signal lml6       : std_logic := '0'; --
  signal lml7       : std_logic := '0'; --
  signal lml8       : std_logic := '0'; --
  signal lml9       : std_logic := '0'; --
  signal lml10      : std_logic := '0'; --
  signal lml12      : std_logic := '0'; --
  signal lml13      : std_logic := '0'; --
  signal zzzv       : std_logic := '0'; --135
  signal yyyv       : std_logic := '0'; --
  signal fffv       : std_logic := '0'; --
  signal zzv        : std_logic := '0'; --
  signal yyv        : std_logic := '0'; --
  signal ffov       : std_logic := '0'; --
  signal fffov      : std_logic := '0'; --
  signal hiev       : std_logic := '0'; --
  signal lumev      : std_logic := '0'; --
  signal c4v        : std_logic := '0'; --
  signal bhabhav    : std_logic := '0'; --
  signal mu_pairv   : std_logic := '0'; --
  signal bha3dv     : std_logic := '0'; --
  signal bffo       : std_logic := '0'; --148
  signal bhie       : std_logic := '0'; --
  signal ffoc       : std_logic := '0'; --
  signal fioiecl1   : std_logic := '0'; --151
  signal fffo       : std_logic := '0'; --
  signal itsf_b2b   : std_logic := '0'; --153
  signal ecleklm1   : std_logic := '0'; --154
  signal ggtsf      : std_logic := '0'; --
  signal ggbrl      : std_logic := '0'; --
  signal bhamtc1    : std_logic := '0'; --
  signal ggsel      : std_logic := '0'; --
  signal y1         : std_logic := '0'; --
  signal y2         : std_logic := '0'; --
  signal yecl       : std_logic := '0'; --
  signal iecl1      : std_logic := '0'; --
  signal secl1      : std_logic := '0'; --
  signal nima01     : std_logic := '0'; -- 
  signal fwd_seklm  : std_logic := '0'; --165
  signal bwd_seklm  : std_logic := '0'; --
  signal klm2       : std_logic := '0'; --
  signal eklm2      : std_logic := '0'; --
  signal beklm      : std_logic := '0'; --
  signal bf         : std_logic := '0'; --170
  signal bs         : std_logic := '0'; --
  signal bz         : std_logic := '0'; --
  signal by         : std_logic := '0'; --
  signal ba         : std_logic := '0'; --
  signal ieklm1     : std_logic := '0'; --
  signal oup176     : std_logic := '0'; --
  signal oup177     : std_logic := '0'; --
  signal oup178     : std_logic := '0'; --
  signal oup179     : std_logic := '0'; --
  signal oup180     : std_logic := '0'; --
  signal oup181     : std_logic := '0'; --
  signal oup182     : std_logic := '0'; --
  signal oup183     : std_logic := '0'; --
  signal oup184     : std_logic := '0'; --
  signal oup185     : std_logic := '0'; --
  signal oup186     : std_logic := '0'; --
  signal oup187     : std_logic := '0'; --
  signal oup188     : std_logic := '0'; --
  signal oup189     : std_logic := '0'; --
  signal oup190     : std_logic := '0'; --
  signal oup191     : std_logic := '0'; --

begin

  VERSION <= ftd_version;
  FTDOUT  <= sig_ftdout; 
          
  --...................................
  --   Inputs
  --...................................

  t3(0)          <= SUBT(iname004'pos(b_t3_0)); --0
  t3(1)          <= SUBT(iname004'pos(b_t3_1));
  t3(2)          <= SUBT(iname004'pos(b_t3_2));
  t3(3)          <= SUBT(iname004'pos(b_t3_3));
  ty(0)          <= SUBT(iname004'pos(b_ty_0));
  ty(1)          <= SUBT(iname004'pos(b_ty_1));
  ty(2)          <= SUBT(iname004'pos(b_ty_2));
  ty(3)          <= SUBT(iname004'pos(b_ty_3));
  t2(0)          <= SUBT(iname004'pos(b_t2_0));
  t2(1)          <= SUBT(iname004'pos(b_t2_1));
  t2(2)          <= SUBT(iname004'pos(b_t2_2));
  t2(3)          <= SUBT(iname004'pos(b_t2_3));
  ts(0)          <= SUBT(iname004'pos(b_ts_0));
  ts(1)          <= SUBT(iname004'pos(b_ts_1));
  ts(2)          <= SUBT(iname004'pos(b_ts_2));
  ts(3)          <= SUBT(iname004'pos(b_ts_3));
  ta(0)          <= SUBT(iname004'pos(b_ta_0));
  ta(1)          <= SUBT(iname004'pos(b_ta_1));
  ta(2)          <= SUBT(iname004'pos(b_ta_2));
  ta(3)          <= SUBT(iname004'pos(b_ta_3));
  cdc_open90     <= SUBT(iname004'pos(b_cdc_open90));
  cdc_active     <= SUBT(iname004'pos(b_cdc_active));
  cdc_b2b3       <= SUBT(iname004'pos(b_cdc_b2b3));
  cdc_b2b5       <= SUBT(iname004'pos(b_cdc_b2b5));
  cdc_b2b7       <= SUBT(iname004'pos(b_cdc_b2b7));
  cdc_b2b9       <= SUBT(iname004'pos(b_cdc_b2b9));
  ehigh          <= SUBT(iname004'pos(b_ehigh));
  elow           <= SUBT(iname004'pos(b_elow));
  elum           <= SUBT(iname004'pos(b_elum));
  ecl_bha        <= SUBT(iname004'pos(b_ecl_bha));
  ecl_3dbha      <= SUBT(iname004'pos(b_ecl_3dbha));
  bha_veto       <= SUBT(iname004'pos(b_bha_veto));
  bha_type(0)    <= SUBT(iname004'pos(b_bha_type_0));
  bha_type(1)    <= SUBT(iname004'pos(b_bha_type_1));
  ti             <= SUBT(iname004'pos(b_ti));
  ecleklm        <= SUBT(iname004'pos(b_ecleklm));
  ieklm          <= SUBT(iname004'pos(b_ieklm));
  fwd_s          <= SUBT(iname004'pos(b_fwd_s));
  bwd_s          <= SUBT(iname004'pos(b_bwd_s));
  bha_intrk      <= SUBT(iname004'pos(b_bha_intrk));
  bha_theta(0)   <= SUBT(iname004'pos(b_bha_theta_0));
  bha_theta(1)   <= SUBT(iname004'pos(b_bha_theta_1));
  tx(0)          <= SUBT(iname004'pos(b_tx_0));
  tx(1)          <= SUBT(iname004'pos(b_tx_1));
  tx(2)          <= SUBT(iname004'pos(b_tx_2));
  tx(3)          <= SUBT(iname004'pos(b_tx_3));
  clst(0)        <= SUBT(iname004'pos(b_clst_0));
  clst(1)        <= SUBT(iname004'pos(b_clst_1));
  clst(2)        <= SUBT(iname004'pos(b_clst_2));
  clst(3)        <= SUBT(iname004'pos(b_clst_3));
  i2fo           <= SUBT(iname004'pos(b_i2fo));
  secl           <= SUBT(iname004'pos(b_secl));
  iecl           <= SUBT(iname004'pos(b_iecl));
  ecl_active     <= SUBT(iname004'pos(b_ecl_active));
  ecl_timing_fwd <= SUBT(iname004'pos(b_ecl_timing_fwd));
  ecl_timing_brl <= SUBT(iname004'pos(b_ecl_timing_brl));
  ecl_timing_bwd <= SUBT(iname004'pos(b_ecl_timing_bwd));
  ecl_phys       <= SUBT(iname004'pos(b_ecl_phys));
  ecl_oflo       <= SUBT(iname004'pos(b_ecl_oflo));
  ecl_lml(0)     <= SUBT(iname004'pos(b_ecl_lml_0));
  ecl_lml(1)     <= SUBT(iname004'pos(b_ecl_lml_1));
  ecl_lml(2)     <= SUBT(iname004'pos(b_ecl_lml_2));
  ecl_lml(3)     <= SUBT(iname004'pos(b_ecl_lml_3));
  ecl_lml(4)     <= SUBT(iname004'pos(b_ecl_lml_4));
  ecl_lml(5)     <= SUBT(iname004'pos(b_ecl_lml_5));
  ecl_lml(6)     <= SUBT(iname004'pos(b_ecl_lml_6));
  ecl_lml(7)     <= SUBT(iname004'pos(b_ecl_lml_7));
  ecl_lml(8)     <= SUBT(iname004'pos(b_ecl_lml_8));
  ecl_lml(9)     <= SUBT(iname004'pos(b_ecl_lml_9));
  ecl_lml(10)    <= SUBT(iname004'pos(b_ecl_lml_10));
  ecl_lml_12     <= SUBT(iname004'pos(b_ecl_lml_12));
  ecl_lml_13     <= SUBT(iname004'pos(b_ecl_lml_13));
  ecl_mumu       <= SUBT(iname004'pos(b_ecl_mumu));
  ecl_bhapur     <= SUBT(iname004'pos(b_ecl_bhapur));
  ecl_bst        <= SUBT(iname004'pos(b_ecl_bst));
  top(0)         <= SUBT(iname004'pos(b_top_0));
  top(1)         <= SUBT(iname004'pos(b_top_1));
  top(2)         <= SUBT(iname004'pos(b_top_2));
  top_bb         <= SUBT(iname004'pos(b_top_bb));
  top_active     <= SUBT(iname004'pos(b_top_active));
  klm_hit        <= SUBT(iname004'pos(b_klm_hit));
  klm(0)         <= SUBT(iname004'pos(b_klm_0));
  klm(1)         <= SUBT(iname004'pos(b_klm_1));
  klm(2)         <= SUBT(iname004'pos(b_klm_2));
  klmb2b         <= SUBT(iname004'pos(b_klmb2b));
  eklm_hit       <= SUBT(iname004'pos(b_eklm_hit));
  eklm(0)        <= SUBT(iname004'pos(b_eklm_0));
  eklm(1)        <= SUBT(iname004'pos(b_eklm_1));
  eklm(2)        <= SUBT(iname004'pos(b_eklm_2));
  eklmb2b        <= SUBT(iname004'pos(b_eklmb2b));
  revo           <= SUBT(iname004'pos(b_revo));
  her_kick       <= SUBT(iname004'pos(b_her_kick));
  ler_kick       <= SUBT(iname004'pos(b_ler_kick));
  bha_delay      <= SUBT(iname004'pos(b_bha_delay));
  pseud_rand     <= SUBT(iname004'pos(b_pseud_rand));
  plsin          <= SUBT(iname004'pos(b_plsin));
  poissonin      <= SUBT(iname004'pos(b_poissonin));
  veto           <= SUBT(iname004'pos(b_veto));
  injv           <= SUBT(iname004'pos(b_injv));
  samhem         <= SUBT(iname004'pos(b_samhem));
  opohem         <= SUBT(iname004'pos(b_opohem));
  n1gev(0)      <= SUBT(iname004'pos(b_n1gev_0));
  n1gev(1)      <= SUBT(iname004'pos(b_n1gev_1));
  n1gev(2)      <= SUBT(iname004'pos(b_n1gev_2));
  n1gev(3)      <= SUBT(iname004'pos(b_n1gev_3));
  n2gev(0)      <= SUBT(iname004'pos(b_n2gev_0));
  n2gev(1)      <= SUBT(iname004'pos(b_n2gev_1));
  n2gev(2)      <= SUBT(iname004'pos(b_n2gev_2));
  n2gev(3)      <= SUBT(iname004'pos(b_n2gev_3));
  c2gev(0)      <= SUBT(iname004'pos(b_c2gev_0));
  c2gev(1)      <= SUBT(iname004'pos(b_c2gev_1));
  c2gev(2)      <= SUBT(iname004'pos(b_c2gev_2));
  c2gev(3)      <= SUBT(iname004'pos(b_c2gev_3));
  cdcecl(0)      <= SUBT(iname004'pos(b_cdcecl_0));
  cdcecl(1)      <= SUBT(iname004'pos(b_cdcecl_1));
  cdcecl(2)      <= SUBT(iname004'pos(b_cdcecl_2));
  cdcecl(3)      <= SUBT(iname004'pos(b_cdcecl_3));
  cdcklm(0)      <= SUBT(iname004'pos(b_cdcklm_0));
  cdcklm(1)      <= SUBT(iname004'pos(b_cdcklm_1));
  seklm(0)       <= SUBT(iname004'pos(b_seklm_0));
  seklm(1)       <= SUBT(iname004'pos(b_seklm_1));
  cdctop(0)      <= SUBT(iname004'pos(b_cdctop_0));
  cdctop(1)      <= SUBT(iname004'pos(b_cdctop_1));
  cdctop(2)      <= SUBT(iname004'pos(b_cdctop_2));
  cdctop(3)      <= SUBT(iname004'pos(b_cdctop_3));
  d3             <= SUBT(iname004'pos(b_d3));
  d5             <= SUBT(iname004'pos(b_d5));
  d7             <= SUBT(iname004'pos(b_d7));
  d9             <= SUBT(iname004'pos(b_d9));
  p3             <= SUBT(iname004'pos(b_p3));
  p5             <= SUBT(iname004'pos(b_p5));
  p7             <= SUBT(iname004'pos(b_p7));
  p9             <= SUBT(iname004'pos(b_p9));
  track          <= SUBT(iname004'pos(b_track));
  trkflt         <= SUBT(iname004'pos(b_trkflt));
  itsfb2b        <= SUBT(iname004'pos(b_itsfb2b));
  f2f30          <= SUBT(iname004'pos(b_f2f30));
  s2f30          <= SUBT(iname004'pos(b_s2f30));
  s2s30          <= SUBT(iname004'pos(b_s2s30));
  s2s3           <= SUBT(iname004'pos(b_s2s3));
  s2s5           <= SUBT(iname004'pos(b_s2s5));
  s2so           <= SUBT(iname004'pos(b_s2so));
  s2f3           <= SUBT(iname004'pos(b_s2f3));
  s2f5           <= SUBT(iname004'pos(b_s2f5));
  s2fo           <= SUBT(iname004'pos(b_s2fo));
  fwdsb          <= SUBT(iname004'pos(b_fwdsb));
  bwdsb          <= SUBT(iname004'pos(b_bwdsb));
  fwdnb          <= SUBT(iname004'pos(b_fwdnb));
  bwdnb          <= SUBT(iname004'pos(b_bwdnb));
  brlfb1         <= SUBT(iname004'pos(b_brlfb1));
  brlfb2         <= SUBT(iname004'pos(b_brlfb2));
  brlnb1         <= SUBT(iname004'pos(b_brlnb1));
  brlnb2         <= SUBT(iname004'pos(b_brlnb2));
  trkbha1        <= SUBT(iname004'pos(b_trkbha1));
  trkbha2        <= SUBT(iname004'pos(b_trkbha2));
  grlgg1         <= SUBT(iname004'pos(b_grlgg1));
  grlgg2         <= SUBT(iname004'pos(b_grlgg2));
  nimin0         <= SUBT(iname004'pos(b_nimin0));
  nimin1         <= SUBT(iname004'pos(b_nimin1));

  --.......................................
  --   FTD logics start
  --.......................................

  fff <= '1' when (t2(2)='1' or t2(3)='1')                                                                                  and veto='0' else '0';
  ffs <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and veto='0' else '0';
  fss <= '1' when (t2(0)='1' or t2(1)='1' or t2(2)='1' or t2(3)='1') and (ts(1)='1' or ts(2)='1' or ts(3)='1')              and veto='0' else '0';
  sss <= '1' when (ts(2)='1' or ts(3)='1')                                                                                  and veto='0' else '0';
  ffz <= '1' when (t2(2)='1' or t2(3)='1')                           and (t3(0)='1' or t3(1)='1' or t3(2)='1' or t3(3)='1') and veto='0' else '0';
  fzz <= '1' when (t2(2)='1' or t2(3)='1')                           and (t3(1)='1' or t3(2)='1' or t3(3)='1')              and veto='0' else '0';
  zzz <= '1' when (t3(2)='1' or t3(3)='1')                                                                                  and veto='0' else '0';
  ffy <= '1' when (t2(2)='1' or t2(3)='1')                           and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and veto='0' else '0';
  fyy <= '1' when (t2(2)='1' or t2(3)='1')                           and (ty(1)='1' or ty(2)='1' or ty(3)='1')              and veto='0' else '0';
  yyy <= '1' when (ty(2)='1' or ty(3)='1')                                                                                  and veto='0' else '0';
  ff  <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')                                                                     and bha_veto='0' and veto='0' else '0';
  fs  <= '1' when (t2(0)='1' or t2(1)='1' or t2(2)='1' or t2(3)='1') and (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and bha_veto='0' and veto='0' else '0';
  ss  <= '1' when (ts(1)='1' or ts(2)='1' or ts(3)='1')                                                                     and bha_veto='0' and veto='0' else '0';
  fz  <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (t3(0)='1' or t3(1)='1' or t3(2)='1' or t3(3)='1') and bha_veto='0' and veto='0' else '0';
  zz  <= '1' when (t3(1)='1' or t3(2)='1' or t3(3)='1')                                                                     and bha_veto='0' and veto='0' else '0';
  fy  <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and bha_veto='0' and veto='0' else '0';
  yy  <= '1' when (ty(1)='1' or ty(2)='1' or ty(3)='1')                                                                     and bha_veto='0' and veto='0' else '0';
  ffo <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')                                                                     and cdc_open90='1' and bha_veto='0' and veto='0' else '0';
  fso <= '1' when (t2(0)='1' or t2(1)='1' or t2(2)='1' or t2(3)='1') and (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and s2fo='1' and bha_veto='0' and veto='0' else '0';
  sso <= '1' when (ts(1)='1' or ts(2)='1' or ts(3)='1')                                                                     and s2so='1' and bha_veto='0' and veto='0' else '0';
  fzo <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (t3(0)='1' or t3(1)='1' or t3(2)='1' or t3(3)='1') and cdc_open90='1' and bha_veto='0' and veto='0' else '0';
  fyo <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and cdc_open90='1' and bha_veto='0' and veto='0' else '0';
  ffb <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')                                                                     and cdc_b2b5='1' and bha_veto='0' and veto='0' else '0';
  fsb <= '1' when (t2(0)='1' or t2(1)='1' or t2(2)='1' or t2(3)='1') and (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and s2f5='1' and bha_veto='0' and veto='0' else '0';
  ssb <= '1' when (ts(1)='1' or ts(2)='1' or ts(3)='1')                                                                     and s2s5='1' and bha_veto='0' and veto='0' else '0';
  fzb <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (t3(0)='1' or t3(1)='1' or t3(2)='1' or t3(3)='1') and cdc_b2b5='1' and bha_veto='0' and veto='0' else '0';
  fyb <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')              and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and cdc_b2b5='1' and bha_veto='0' and veto='0' else '0';
  syo <= '1' when (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and s2fo='1'     and bha_veto='0' and veto='0' else '0';
  syb <= '1' when (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and s2f5='1'     and bha_veto='0' and veto='0' else '0';
  ff30 <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1')                                                                     and f2f30='1' and bha_veto='0' and veto='0' else '0';
  fs30 <= '1' when (t2(0)='1' or t2(1)='1' or t2(2)='1' or t2(3)='1') and (ts(0)='1' or ts(1)='1' or ts(2)='1' or ts(3)='1') and s2f30='1' and bha_veto='0' and veto='0' else '0';
  ss30 <= '1' when (ts(1)='1' or ts(2)='1' or ts(3)='1')                                                                     and s2s30='1' and bha_veto='0' and veto='0' else '0';
  fy30 <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1') and (ty(0)='1' or ty(1)='1' or ty(2)='1' or ty(3)='1') and f2f30='1' and bha_veto='0' and veto='0' else '0';
  aaa  <= '1' when (ta(2)='1' or ta(3)='1')                                                  and veto='0' else '0';
  aaao <= '1' when (ta(2)='1' or ta(3)='1')              and cdc_open90='1'                  and veto='0' else '0';
  aao  <= '1' when (ta(1)='1' or ta(2)='1' or ta(3)='1') and cdc_open90='1' and bha_veto='0' and veto='0' else '0';
  aab  <= '1' when (ta(1)='1' or ta(2)='1' or ta(3)='1') and cdc_b2b5='1'   and bha_veto='0' and veto='0' else '0';
  aa   <= '1' when (ta(1)='1' or ta(2)='1' or ta(3)='1')                    and bha_veto='0' and veto='0' else '0';

  hie        <= '1' when ehigh='1' and bha_veto='0' and veto='0' else '0';
  lowe       <= '1' when elow='1'  and bha_veto='0' and veto='0' else '0';
  lume       <= '1' when elum='1'                   and veto='0' else '0';
  hade       <= '1' when elum='1'  and bha_veto='0' and veto='0' else '0';
  c2         <= '1' when clst>1    and bha_veto='0' and veto='0' else '0';
  c3         <= '1' when clst>2    and bha_veto='0' and veto='0' else '0';
  c4         <= '1' when clst>3    and bha_veto='0' and veto='0' else '0';
  c5         <= '1' when clst>4    and bha_veto='0' and veto='0' else '0';
  bha3d      <= '1' when ecl_3dbha='1'             and veto='0' else '0';
  bhabha     <= '1' when ecl_bha='1'               and veto='0' else '0';
  bhapur     <= '1' when ecl_bhapur='1' and veto='0' else '0';
  bha_id0    <= '1' when ecl_bhapur='1' and bha_theta=3 and veto='0' else '0';
  bha_id1    <= '1' when ecl_bhapur='1' and bha_theta=0 and veto='0' else '0';
  bha_id2    <= '1' when ecl_bhapur='1' and bha_theta=1 and veto='0' else '0'; 
  bha_id3    <= '1' when ecl_bhapur='1' and bha_theta=2 and veto='0' else '0'; 
  eclmumu    <= '1' when ecl_mumu='1'   and bha_veto='0' and veto='0' else '0';
  ecloflo    <= '1' when ecl_oflo='1'   and veto='0' else '0';
  eclbst     <= '1' when ecl_bst='1' and veto='0' else '0';
  g_high     <= '1' when ehigh='1'  and track='0' and veto='0' else '0';
  g_c1       <= '1' when clst=1     and track='0' and veto='0' else '0';
  gg         <= '1' when clst=2     and track='0' and veto='0' else '0';

  eed    <= '1' when t2(1)='1' and cdcecl(1)='1' and d5='1' and bha_veto='0' and veto='0' else '0';
  fed    <= '1' when t2(0)='1' and cdcecl(0)='1' and d5='1' and bha_veto='0' and veto='0' else '0';
  fp     <= '1' when t2(0)='1' and                   p5='1' and bha_veto='0' and veto='0' else '0';
  sp     <= '1' when ts(0)='1' and                   p5='1' and bha_veto='0' and veto='0' else '0';
  zp     <= '1' when t3(0)='1' and                   p5='1' and bha_veto='0' and veto='0' else '0';
  yp     <= '1' when ty(0)='1' and                   p5='1' and bha_veto='0' and veto='0' else '0';
  d_5    <= '1' when d5='1'                                 and bha_veto='0' and veto='0' else '0';
  shem   <= '1' when samhem='1' and                 veto='0' else '0';
  ohem   <= '1' when opohem='1' and bha_veto='0' and veto='0' else '0';

  toptiming <= '1' when top_active='1' and veto='0' else '0';
  ecltiming <= '1' when ecl_active='1' and veto='0' else '0';
  cdctiming <= '1' when cdc_active='1' and veto='0' else '0';

  cdcbb     <= '1' when cdc_b2b5='1' and bha_veto='0' and veto='0' else '0';
  mu_pair    <= '1' when klm>0    and (t2(1)='1' or t2(2)='1' or t2(3)='1') and veto='0' else '0';
  mu_b2b     <= '1' when klmb2b='1' and veto='0' else '0';
  klmhit     <= '1' when klm_hit='1'  and veto='0' else '0';
  mu_epair   <= '1' when eklm>0       and veto='0' else '0';
  mu_eb2b    <= '1' when eklmb2b='1'  and veto='0' else '0';
  eklmhit    <= '1' when eklm_hit='1' and veto='0' else '0';

  revolution <= '1' when revo='1'       and veto='0' else '0';
  random     <= '1' when pseud_rand='1' and veto='0' else '0';
  bg         <= '1' when bha_delay='1'  and veto='0' else '0';
  pls        <= '1' when plsin='1'      and veto='0' else '0';
  poisson    <= '1' when poissonin='1'  and veto='0' else '0';
  vetout     <= '1' when                    veto='1' else '0';

  f <= '1' when t2>0 and bha_veto='0' and veto='0' else '0';
  s <= '1' when ts>0 and bha_veto='0' and veto='0' else '0';
  z <= '1' when t3>0 and bha_veto='0' and veto='0' else '0';
  y <= '1' when ty>0 and bha_veto='0' and veto='0' else '0';
  a <= '1' when ta>0 and bha_veto='0' and veto='0' else '0';

  n1gev1  <= '1' when n1gev(0)='1' and veto='0' else '0';
  n1gev2  <= '1' when n1gev(1)='1' and veto='0' else '0';
  n1gev3  <= '1' when n1gev(2)='1' and veto='0' else '0';
  n1gev4  <= '1' when n1gev(3)='1' and veto='0' else '0';
  n2gev1  <= '1' when n2gev(0)='1' and veto='0' else '0';
  n2gev2  <= '1' when n2gev(1)='1' and veto='0' else '0';
  n2gev3  <= '1' when n2gev(2)='1' and veto='0' else '0';
  n2gev4  <= '1' when n2gev(3)='1' and veto='0' else '0';
  c2gev1  <= '1' when c2gev(0)='1' and bha_veto='0' and veto='0' else '0';
  c2gev2  <= '1' when c2gev(1)='1' and bha_veto='0' and veto='0' else '0';
  c2gev3  <= '1' when c2gev(2)='1' and veto='0' else '0';
  c2gev4  <= '1' when c2gev(3)='1' and veto='0' else '0';
  cdcecl1 <= '1' when cdcecl(0)='1' and bha_veto='0' and veto='0' else '0';
  cdcecl2 <= '1' when cdcecl(1)='1' and bha_veto='0' and veto='0' else '0';
  cdcecl3 <= '1' when cdcecl(2)='1' and veto='0' else '0';
  cdcecl4 <= '1' when cdcecl(3)='1' and veto='0' else '0';
  cdcklm1 <= '1' when cdcklm(0)='1' and bha_veto='0' and veto='0' else '0';
  cdcklm2 <= '1' when cdcklm(1)='1' and bha_veto='0' and veto='0' else '0';
  seklm1  <= '1' when seklm(0)='1'  and bha_veto='0' and veto='0' else '0';
  seklm2  <= '1' when seklm(1)='1'  and bha_veto='0' and veto='0' else '0';
  cdctop1 <= '1' when cdctop(0)='1' and bha_veto='0' and veto='0' else '0';
  cdctop2 <= '1' when cdctop(1)='1' and bha_veto='0' and veto='0' else '0';
  cdctop3 <= '1' when cdctop(2)='1' and veto='0' else '0';
  cdctop4 <= '1' when cdctop(3)='1' and veto='0' else '0';
  c1hie  <= '1' when clst=1 and ehigh='1' and veto='0' else '0';
  c1lume <= '1' when clst=1 and elum='1'  and veto='0' else '0';
  n1hie  <= '1' when clst=1 and ehigh='1' and track='0' and veto='0' else '0';
  n1lume <= '1' when clst=1 and elum='1'  and track='0' and veto='0' else '0';
  c3hie  <= '1' when clst=3 and ehigh='1' and veto='0' else '0';
  c3lume <= '1' when clst=3 and elum='1'  and veto='0' else '0';
  n3hie  <= '1' when clst=3 and ehigh='1' and track='0' and veto='0' else '0';
  n3lume <= '1' when clst=3 and elum='1'  and track='0' and veto='0' else '0';
  c2hie  <= '1' when clst>1 and ehigh='1' and bha_veto='0' and veto='0' else '0';

  lml0   <= '1' when ecl_lml(0)='1' and veto='0' else '0';
  lml1   <= '1' when ecl_lml(1)='1' and veto='0' else '0';
  lml2   <= '1' when ecl_lml(2)='1' and veto='0' else '0';
  lml3   <= '1' when ecl_lml(3)='1' and veto='0' else '0';
  lml4   <= '1' when ecl_lml(4)='1' and veto='0' else '0';
  lml5   <= '1' when ecl_lml(5)='1' and veto='0' else '0';
  lml6   <= '1' when ecl_lml(6)='1' and veto='0' else '0';
  lml7   <= '1' when ecl_lml(7)='1' and veto='0' else '0';
  lml8   <= '1' when ecl_lml(8)='1' and veto='0' else '0';
  lml9   <= '1' when ecl_lml(9)='1' and veto='0' else '0';
  lml10  <= '1' when ecl_lml(10)='1' and veto='0' else '0';
  lml12  <= '1' when ecl_lml_12='1' and veto='0' else '0';
  lml13  <= '1' when ecl_lml_13='1' and veto='0' else '0';

  zzzv     <= '1' when (t3(2)='1' or t3(3)='1')                                                   else '0';
  yyyv     <= '1' when (ty(2)='1' or ty(3)='1')                                                   else '0';
  fffv     <= '1' when (t2(2)='1' or t2(3)='1')                                                   else '0';
  zzv      <= '1' when (t3(1)='1' or t3(2)='1' or t3(3)='1')                    and bha_veto='0' else '0';
  yyv      <= '1' when (ty(1)='1' or ty(2)='1' or ty(3)='1')                    and bha_veto='0' else '0';
  ffov     <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1') and cdc_open90='1' and bha_veto='0' else '0';
  fffov    <= '1' when (t2(2)='1' or t2(3)='1')                         and cdc_open90='1' else '0';
  hiev     <= '1' when ehigh='1'                                                and bha_veto='0' else '0';
  lumev    <= '1' when elum='1'                                                                  else '0';
  c4v      <= '1' when clst>3                                                 and bha_veto='0' else '0';
  bhabhav  <= '1' when ecl_bha='1'                                                               else '0';
  mu_pairv <= '1' when klm>0    and (t2(1)='1' or t2(2)='1' or t2(3)='1')                       else '0';
  bha3dv   <= '1' when ecl_3dbha='1'                                                              else '0';

  bffo  <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1') and cdc_open90='1' and veto='0' else '0';
  bhie  <= '1' when ehigh='1'                                                and veto='0' else '0';
  ffoc  <= '1' when (t2(1)='1' or t2(2)='1' or t2(3)='1') and clst>0 and cdc_open90='1' and bha_veto='0' and veto='0' else '0';
  fioiecl1 <= '1' when t2>0 and ti='1' and i2fo='1' and iecl='1' and bha_veto='0' and veto='0' else '0';
  fffo  <= '1' when (t2(2)='1' or t2(3)='1')              and cdc_open90='1' and veto='0' else '0';

  itsf_b2b <= '1' when itsfb2b='1' and veto='0' else '0';

  ecleklm1 <= '1' when ecleklm='1' and bha_veto='0' and veto='0' else '0';
  ggtsf   <= '1' when grlgg2='1' and bha_intrk='1' and veto='0' else '0';
  ggbrl   <= '1' when brlnb2='1'                   and veto='0' else '0';
  bhamtc1 <= '1' when (trkbha1='1' or trkbha2='1') and veto='0' else '0';
  ggsel   <= '1' when ecl_3dbha='1' and (trkbha1='0' and trkbha2='0') and bha_intrk='1' and veto='0' else '0';
  
  y1  <= '1' when ty(0)='1' and bha_veto='0' and veto='0' else '0';
  y2  <= '1' when ty(1)='1' and bha_veto='0' and veto='0' else '0';
  yecl <= '1' when ty>0 and cdcecl>0 and bha_veto='0' and veto='0' else '0';
  iecl1 <= '1' when iecl='1'                              and bha_veto='0'   and veto='0' else '0';
  secl1 <= '1' when secl='1'                              and bha_veto='0'   and veto='0' else '0';

  nima01 <= '1' when nimin0='1' and nimin1='1' and veto='0' else '0';

  fwd_seklm <= '1' when fwd_s='1' and eklm_hit='1' and veto='0' else '0';
  bwd_seklm <= '1' when bwd_s='1' and eklm_hit='1' and veto='0' else '0';
  klm2     <= '1' when  klm>1           and bha_veto='0' and veto='0' else '0';
  eklm2    <= '1' when eklm>1           and bha_veto='0' and veto='0' else '0';
  beklm    <= '1' when eklm=1 and klm=1 and bha_veto='0' and veto='0' else '0';

  bf <= '1' when t2>0 and veto='0' else '0';
  bs <= '1' when ts>0 and veto='0' else '0';
  bz <= '1' when t3>0 and veto='0' else '0';
  by <= '1' when ty>0 and veto='0' else '0';
  ba <= '1' when ta>0 and veto='0' else '0';

  ieklm1 <= '1' when ieklm='1' and bha_veto='0' and veto='0' else '0';

  --.......................................
  --   FTD logics end
  --.......................................

  process (clk)
  begin
    if (rising_edge(clk)) then
      sig_ftdout(oname004'pos(b_fff)) <= fff;
      sig_ftdout(oname004'pos(b_ffs)) <= ffs;
      sig_ftdout(oname004'pos(b_fss)) <= fss;
      sig_ftdout(oname004'pos(b_sss)) <= sss;
      sig_ftdout(oname004'pos(b_ffz)) <= ffz;
      sig_ftdout(oname004'pos(b_fzz)) <= fzz;
      sig_ftdout(oname004'pos(b_zzz)) <= zzz;
      sig_ftdout(oname004'pos(b_ffy)) <= ffy;
      sig_ftdout(oname004'pos(b_fyy)) <= fyy;
      sig_ftdout(oname004'pos(b_yyy)) <= yyy;
      sig_ftdout(oname004'pos(b_ff)) <= ff;
      sig_ftdout(oname004'pos(b_fs)) <= fs;
      sig_ftdout(oname004'pos(b_ss)) <= ss;
      sig_ftdout(oname004'pos(b_fz)) <= fz;
      sig_ftdout(oname004'pos(b_zz)) <= zz;
      sig_ftdout(oname004'pos(b_fy)) <= fy;
      sig_ftdout(oname004'pos(b_yy)) <= yy;
      sig_ftdout(oname004'pos(b_ffo)) <= ffo;
      sig_ftdout(oname004'pos(b_fso)) <= fso;
      sig_ftdout(oname004'pos(b_sso)) <= sso;
      sig_ftdout(oname004'pos(b_fzo)) <= fzo;
      sig_ftdout(oname004'pos(b_fyo)) <= fyo;
      sig_ftdout(oname004'pos(b_ffb)) <= ffb;
      sig_ftdout(oname004'pos(b_fsb)) <= fsb;
      sig_ftdout(oname004'pos(b_ssb)) <= ssb;
      sig_ftdout(oname004'pos(b_fzb)) <= fzb;
      sig_ftdout(oname004'pos(b_fyb)) <= fyb;
      sig_ftdout(oname004'pos(b_syo)) <= syo;
      sig_ftdout(oname004'pos(b_syb)) <= syb;
      sig_ftdout(oname004'pos(b_ff30)) <= ff30;
      sig_ftdout(oname004'pos(b_fs30)) <= fs30;
      sig_ftdout(oname004'pos(b_ss30)) <= ss30;
      sig_ftdout(oname004'pos(b_fy30)) <= fy30;
      sig_ftdout(oname004'pos(b_aaa)) <= aaa;
      sig_ftdout(oname004'pos(b_aaao)) <= aaao;
      sig_ftdout(oname004'pos(b_aao)) <= aao;
      sig_ftdout(oname004'pos(b_aab)) <= aab;
      sig_ftdout(oname004'pos(b_aa)) <= aa;

      sig_ftdout(oname004'pos(b_hie)) <= hie;
      sig_ftdout(oname004'pos(b_lowe)) <= lowe;
      sig_ftdout(oname004'pos(b_lume)) <= lume;
      sig_ftdout(oname004'pos(b_hade)) <= hade;
      sig_ftdout(oname004'pos(b_c2)) <= c2;
      sig_ftdout(oname004'pos(b_c3)) <= c3;
      sig_ftdout(oname004'pos(b_c4)) <= c4;
      sig_ftdout(oname004'pos(b_c5)) <= c5;
      sig_ftdout(oname004'pos(b_bha3d)) <= bha3d;
      sig_ftdout(oname004'pos(b_bhabha)) <= bhabha;
      sig_ftdout(oname004'pos(b_bhapur)) <= bhapur;
      sig_ftdout(oname004'pos(b_bha_id0)) <= bha_id0;
      sig_ftdout(oname004'pos(b_bha_id1)) <= bha_id1;
      sig_ftdout(oname004'pos(b_bha_id2)) <= bha_id2;
      sig_ftdout(oname004'pos(b_bha_id3)) <= bha_id3;
      sig_ftdout(oname004'pos(b_eclmumu)) <= eclmumu;
      sig_ftdout(oname004'pos(b_ecloflo)) <= ecloflo;
      sig_ftdout(oname004'pos(b_eclbst)) <= eclbst;
      sig_ftdout(oname004'pos(b_g_high)) <= g_high;
      sig_ftdout(oname004'pos(b_g_c1)) <= g_c1;
      sig_ftdout(oname004'pos(b_gg)) <= gg;

      sig_ftdout(oname004'pos(b_eed)) <= eed;
      sig_ftdout(oname004'pos(b_fed)) <= fed;
      sig_ftdout(oname004'pos(b_fp)) <= fp;
      sig_ftdout(oname004'pos(b_sp)) <= sp;
      sig_ftdout(oname004'pos(b_zp)) <= zp;
      sig_ftdout(oname004'pos(b_yp)) <= yp;
      sig_ftdout(oname004'pos(b_d_5)) <= d_5;
      sig_ftdout(oname004'pos(b_shem)) <= shem;
      sig_ftdout(oname004'pos(b_ohem)) <= ohem;

      sig_ftdout(oname004'pos(b_toptiming)) <= toptiming;
      sig_ftdout(oname004'pos(b_ecltiming)) <= ecltiming;
      sig_ftdout(oname004'pos(b_cdctiming)) <= cdctiming;

      sig_ftdout(oname004'pos(b_cdcbb)) <= cdcbb;
      sig_ftdout(oname004'pos(b_mu_pair)) <= mu_pair;
      sig_ftdout(oname004'pos(b_mu_b2b)) <= mu_b2b;
      sig_ftdout(oname004'pos(b_klmhit)) <= klmhit;
      sig_ftdout(oname004'pos(b_mu_epair)) <= mu_epair;
      sig_ftdout(oname004'pos(b_mu_eb2b)) <= mu_eb2b;
      sig_ftdout(oname004'pos(b_eklmhit)) <= eklmhit;

      sig_ftdout(oname004'pos(b_revolution)) <= revolution;
      sig_ftdout(oname004'pos(b_random)) <= random;
      sig_ftdout(oname004'pos(b_bg)) <= bg;
      sig_ftdout(oname004'pos(b_pls)) <= pls;
      sig_ftdout(oname004'pos(b_poisson)) <= poisson;
      sig_ftdout(oname004'pos(b_vetout)) <= vetout;

      sig_ftdout(oname004'pos(b_f)) <= f;
      sig_ftdout(oname004'pos(b_s)) <= s;
      sig_ftdout(oname004'pos(b_z)) <= z;
      sig_ftdout(oname004'pos(b_y)) <= y;
      sig_ftdout(oname004'pos(b_a)) <= a;

      sig_ftdout(oname004'pos(b_n1gev1)) <= n1gev1;
      sig_ftdout(oname004'pos(b_n1gev2)) <= n1gev2;
      sig_ftdout(oname004'pos(b_n1gev3)) <= n1gev3;
      sig_ftdout(oname004'pos(b_n1gev4)) <= n1gev4;
      sig_ftdout(oname004'pos(b_n2gev1)) <= n2gev1;
      sig_ftdout(oname004'pos(b_n2gev2)) <= n2gev2;
      sig_ftdout(oname004'pos(b_n2gev3)) <= n2gev3;
      sig_ftdout(oname004'pos(b_n2gev4)) <= n2gev4;
      sig_ftdout(oname004'pos(b_c2gev1)) <= c2gev1;
      sig_ftdout(oname004'pos(b_c2gev2)) <= c2gev2;
      sig_ftdout(oname004'pos(b_c2gev3)) <= c2gev3;
      sig_ftdout(oname004'pos(b_c2gev4)) <= c2gev4;
      sig_ftdout(oname004'pos(b_cdcecl1)) <= cdcecl1;
      sig_ftdout(oname004'pos(b_cdcecl2)) <= cdcecl2;
      sig_ftdout(oname004'pos(b_cdcecl3)) <= cdcecl3;
      sig_ftdout(oname004'pos(b_cdcecl4)) <= cdcecl4;
      sig_ftdout(oname004'pos(b_cdcklm1)) <= cdcklm1;
      sig_ftdout(oname004'pos(b_cdcklm2)) <= cdcklm2;
      sig_ftdout(oname004'pos(b_seklm1)) <= seklm1;
      sig_ftdout(oname004'pos(b_seklm2)) <= seklm2;
      sig_ftdout(oname004'pos(b_cdctop1)) <= cdctop1;
      sig_ftdout(oname004'pos(b_cdctop2)) <= cdctop2;
      sig_ftdout(oname004'pos(b_cdctop3)) <= cdctop3;
      sig_ftdout(oname004'pos(b_cdctop4)) <= cdctop4;
      sig_ftdout(oname004'pos(b_c1hie)) <= c1hie;
      sig_ftdout(oname004'pos(b_c1lume)) <= c1lume;
      sig_ftdout(oname004'pos(b_n1hie)) <= n1hie;
      sig_ftdout(oname004'pos(b_n1lume)) <= n1lume;
      sig_ftdout(oname004'pos(b_c3hie)) <= c3hie;
      sig_ftdout(oname004'pos(b_c3lume)) <= c3lume;
      sig_ftdout(oname004'pos(b_n3hie)) <= n3hie;
      sig_ftdout(oname004'pos(b_n3lume)) <= n3lume;
      sig_ftdout(oname004'pos(b_c2hie)) <= c2hie;

      sig_ftdout(oname004'pos(b_lml0)) <= lml0;
      sig_ftdout(oname004'pos(b_lml1)) <= lml1;
      sig_ftdout(oname004'pos(b_lml2)) <= lml2;
      sig_ftdout(oname004'pos(b_lml3)) <= lml3;
      sig_ftdout(oname004'pos(b_lml4)) <= lml4;
      sig_ftdout(oname004'pos(b_lml5)) <= lml5;
      sig_ftdout(oname004'pos(b_lml6)) <= lml6;
      sig_ftdout(oname004'pos(b_lml7)) <= lml7;
      sig_ftdout(oname004'pos(b_lml8)) <= lml8;
      sig_ftdout(oname004'pos(b_lml9)) <= lml9;
      sig_ftdout(oname004'pos(b_lml10)) <= lml10;
      sig_ftdout(oname004'pos(b_lml12)) <= lml12;
      sig_ftdout(oname004'pos(b_lml13)) <= lml13;

      sig_ftdout(oname004'pos(b_zzzv)) <= zzzv;
      sig_ftdout(oname004'pos(b_yyyv)) <= yyyv;
      sig_ftdout(oname004'pos(b_fffv)) <= fffv;
      sig_ftdout(oname004'pos(b_zzv)) <= zzv;
      sig_ftdout(oname004'pos(b_yyv)) <= yyv;
      sig_ftdout(oname004'pos(b_ffov)) <= ffov;
      sig_ftdout(oname004'pos(b_fffov)) <= fffov;
      sig_ftdout(oname004'pos(b_hiev)) <= hiev;
      sig_ftdout(oname004'pos(b_lumev)) <= lumev;
      sig_ftdout(oname004'pos(b_c4v)) <= c4v;
      sig_ftdout(oname004'pos(b_bhabhav)) <= bhabhav;
      sig_ftdout(oname004'pos(b_mu_pairv)) <= mu_pairv;
      sig_ftdout(oname004'pos(b_bha3dv)) <= bha3dv;

      sig_ftdout(oname004'pos(b_bffo)) <= bffo;
      sig_ftdout(oname004'pos(b_bhie)) <= bhie;
      sig_ftdout(oname004'pos(b_ffoc)) <= ffoc;
      sig_ftdout(oname004'pos(b_fioiecl1)) <= fioiecl1;
      sig_ftdout(oname004'pos(b_fffo))  <= fffo;

      sig_ftdout(oname004'pos(b_itsf_b2b)) <= itsf_b2b;

      sig_ftdout(oname004'pos(b_ecleklm1)) <= ecleklm1;
      sig_ftdout(oname004'pos(b_ggtsf)) <= ggtsf;
      sig_ftdout(oname004'pos(b_ggbrl)) <= ggbrl;
      sig_ftdout(oname004'pos(b_bhamtc1)) <= bhamtc1;
      sig_ftdout(oname004'pos(b_ggsel)) <= ggsel;
      sig_ftdout(oname004'pos(b_y1)) <= y1;
      sig_ftdout(oname004'pos(b_y2)) <= y2;
      sig_ftdout(oname004'pos(b_yecl)) <= yecl;
      sig_ftdout(oname004'pos(b_iecl1)) <= iecl1;
      sig_ftdout(oname004'pos(b_secl1)) <= secl1;

      sig_ftdout(oname004'pos(b_nima01)) <= nima01;

      sig_ftdout(oname004'pos(b_fwd_seklm)) <= fwd_seklm;
      sig_ftdout(oname004'pos(b_bwd_seklm)) <= bwd_seklm;
      sig_ftdout(oname004'pos(b_klm2))      <= klm2;
      sig_ftdout(oname004'pos(b_eklm2))     <= eklm2;
      sig_ftdout(oname004'pos(b_beklm))     <= beklm;

      sig_ftdout(oname004'pos(b_bf)) <= bf;
      sig_ftdout(oname004'pos(b_bs)) <= bs;
      sig_ftdout(oname004'pos(b_bz)) <= bz;
      sig_ftdout(oname004'pos(b_by)) <= by;
      sig_ftdout(oname004'pos(b_ba)) <= ba;
      sig_ftdout(oname004'pos(b_ieklm1)) <= ieklm1;

    end if;
  end process;
end RTL;
