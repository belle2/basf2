/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGGRLUNPACKER_H
#define TRGGRLUNPACKER_H

#include <string>
#include <vector>

#include "rawdata/dataobjects/RawTRG.h"
#include "trg/grl/dataobjects/TRGGRLUnpackerStore.h"

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  namespace TRGGRLUNPACKERSPACE {

    /** num of leafs in data_b2l **/
    const int nLeafs = 128 + 3 + 32 + 7;
    /** num of leafs for others **/
    const int nLeafsExtra = 7;
    /** num of clk time window **/
    const int nClks = 1 ;
    /** num of b2l bits **/
    const int nBits = 2048;
    /** leaf names **/
    const char* LeafNames[nLeafs + nLeafsExtra] = {
      "2GeV_15",
      "2GeV_14",
      "2GeV_13",
      "2GeV_12",
      "2GeV_11",
      "2GeV_10",
      "2GeV_9",
      "2GeV_8",
      "2GeV_7",
      "2GeV_6",
      "2GeV_5",
      "2GeV_4",
      "2GeV_3",
      "2GeV_2",
      "2GeV_1",
      "2GeV_0",
      "1GeV_15",
      "1GeV_14",
      "1GeV_13",
      "1GeV_12",
      "1GeV_11",
      "1GeV_10",
      "1GeV_9",
      "1GeV_8",
      "1GeV_7",
      "1GeV_6",
      "1GeV_5",
      "1GeV_4",
      "1GeV_3",
      "1GeV_2",
      "1GeV_1",
      "1GeV_0",
      "wire_tsf0",
      "wire_tsf2",
      "wire_tsf4",
      "wire_tsf1",
      "wire_tsf3",
      "theta_11",
      "phi_11",
      "t_11",
      "E_11",
      "theta_10",
      "phi_10",
      "t_10",
      "E_10",
      "theta_9",
      "phi_9",
      "t_9",
      "E_9",
      "theta_8",
      "phi_8",
      "t_8",
      "E_8",
      "theta_7",
      "phi_7",
      "t_7",
      "E_7",
      "theta_6",
      "phi_6",
      "t_6",
      "E_6",
      "X_1",
      "N_cluster_1",
      "ECL_trg_1",
      "theta_5",
      "phi_5",
      "t_5",
      "E_5",
      "theta_4",
      "phi_4",
      "t_4",
      "E_4",
      "theta_3",
      "phi_3",
      "t_3",
      "E_3",
      "theta_2",
      "phi_2",
      "t_2",
      "E_2",
      "theta_1",
      "phi_1",
      "t_1",
      "E_1",
      "theta_0",
      "phi_0",
      "t_0",
      "E_0",
      "X_0",
      "N_cluster_0",
      "ECL_trg_0",
      "N_track_cut",
      "N_track_3D",
      "N_track_NN",
      "CDCL1_count",
      "phi_CDC_int_1",
      "phi_CDC_int_0",
      "phi_i_int_1",
      "phi_i_int_0",
      "N_track",
      "b2b_1to3",
      "b2b_1to5",
      "b2b_1to7",
      "b2b_1to9",
      "opening",
      "samehem_1",
      "opphem_1",
      "c_b2b_9_1",
      "c_b2b_7_1",
      "c_b2b_5_1",
      "c_b2b_3_1",
      "tc_b2b_9_1",
      "tc_b2b_7_1",
      "tc_b2b_5_1",
      "tc_b2b_3_1",
      "TOPL1_count",
      "KLML1_count",
      "sector_KLM_int",
      "sector_CDC_int",
      "N_KLMmatch",
      "samehem_0",
      "opphem_0",
      "N_match_2GeV_0",
      "N_match_0",
      "N_g_1GeV_0",
      "N_g_2GeV_0",
      "ECLL1_count",
      "c_b2b_9_0",
      "c_b2b_7_0",
      "c_b2b_5_0",
      "c_b2b_3_0",
      "tc_b2b_9_0",
      "tc_b2b_7_0",
      "tc_b2b_5_0",
      "tc_b2b_3_0",
      "CDC3DL1_count",
      "CDCNNL1_count",
      "slot_CDC_int",
      "slot_TOP_int",
      "N_TOPmatch",
      "N_match_2GeV_1",
      "N_match_1",
      "N_g_1GeV_1",
      "N_g_2GeV_1",
      "TSFL1_count",
      "slot64b_TOP_int_1",
      "slot64b_TOP_int_0",
      "map_TSF3_int_1",
      "map_TSF3_int_0",
      "map_TSF1_int_1",
      "map_TSF1_int_0",
      "map_N_ST",
      "map_ST2_int_1",
      "map_ST2_int_0",
      "map_ST_int_1",
      "map_ST_int_0",
      "map_veto_int_1",
      "map_veto_int_0",
      "map_TSF4_int_1",
      "map_TSF4_int_0",
      "map_TSF2_int_1",
      "map_TSF2_int_0",
      "map_TSF0_int_1",
      "map_TSF0_int_0",
      "charge_flag",
      "charge_sum",
      "revoclk",
      "aaaa",
      "firmid", "firmver", "evt", "clk", "coml1", "b2ldly", "maxrvc"
    };


    /** enum of leafs **/
    enum EBits {
      e_2GeV_15,
      e_2GeV_14,
      e_2GeV_13,
      e_2GeV_12,
      e_2GeV_11,
      e_2GeV_10,
      e_2GeV_9,
      e_2GeV_8,
      e_2GeV_7,
      e_2GeV_6,
      e_2GeV_5,
      e_2GeV_4,
      e_2GeV_3,
      e_2GeV_2,
      e_2GeV_1,
      e_2GeV_0,
      e_1GeV_15,
      e_1GeV_14,
      e_1GeV_13,
      e_1GeV_12,
      e_1GeV_11,
      e_1GeV_10,
      e_1GeV_9,
      e_1GeV_8,
      e_1GeV_7,
      e_1GeV_6,
      e_1GeV_5,
      e_1GeV_4,
      e_1GeV_3,
      e_1GeV_2,
      e_1GeV_1,
      e_1GeV_0,
      e_wire_tsf0,
      e_wire_tsf2,
      e_wire_tsf4,
      e_wire_tsf1,
      e_wire_tsf3,
      e_theta_11,
      e_phi_11,
      e_t_11,
      e_E_11,
      e_theta_10,
      e_phi_10,
      e_t_10,
      e_E_10,
      e_theta_9,
      e_phi_9,
      e_t_9,
      e_E_9,
      e_theta_8,
      e_phi_8,
      e_t_8,
      e_E_8,
      e_theta_7,
      e_phi_7,
      e_t_7,
      e_E_7,
      e_theta_6,
      e_phi_6,
      e_t_6,
      e_E_6,
      e_X_1,
      e_N_cluster_1,
      e_ECL_trg_1,
      e_theta_5,
      e_phi_5,
      e_t_5,
      e_E_5,
      e_theta_4,
      e_phi_4,
      e_t_4,
      e_E_4,
      e_theta_3,
      e_phi_3,
      e_t_3,
      e_E_3,
      e_theta_2,
      e_phi_2,
      e_t_2,
      e_E_2,
      e_theta_1,
      e_phi_1,
      e_t_1,
      e_E_1,
      e_theta_0,
      e_phi_0,
      e_t_0,
      e_E_0,
      e_X_0,
      e_N_cluster_0,
      e_ECL_trg_0,
      e_ECL_trg,
      e_ECL_trg_delay,
      e_N_track_cut,
      e_N_track_NN,
      e_N_track_3D,
      e_CDCL1_count,
      e_phi_CDC_int_1,
      e_phi_CDC_int_0,
      e_phi_i_int_1,
      e_phi_i_int_0,
      e_N_track,
      e_b2b_1to3,
      e_b2b_1to5,
      e_b2b_1to7,
      e_b2b_1to9,
      e_opening,
      e_samehem_1,
      e_opphem_1,
      e_c_b2b_1to9_1,
      e_c_b2b_1to7_1,
      e_c_b2b_1to5_1,
      e_c_b2b_1to3_1,
      e_tc_b2b_1to9_1,
      e_tc_b2b_1to7_1,
      e_tc_b2b_1to5_1,
      e_tc_b2b_1to3_1,
      e_TOPL1_count,
      e_KLML1_count,
      e_sector_KLM_int,
      e_sector_CDC_int,
      e_N_KLMmatch,
      e_samehem_0,
      e_opphem_0,
      e_N_match_2GeV_0,
      e_N_match_0,
      e_N_g_1GeV_0,
      e_N_g_2GeV_0,
      e_ECLL1_count,
      e_c_b2b_1to9_0,
      e_c_b2b_1to7_0,
      e_c_b2b_1to5_0,
      e_c_b2b_1to3_0,
      e_tc_b2b_1to9_0,
      e_tc_b2b_1to7_0,
      e_tc_b2b_1to5_0,
      e_tc_b2b_1to3_0,
      e_CDC3DL1_count,
      e_CDCNNL1_count,
      e_slot_CDC_int,
      e_slot_TOP_int,
      e_N_TOPmatch,
      e_N_match_2GeV_1,
      e_N_match_1,
      e_N_g_1GeV_1,
      e_N_g_2GeV_1,
      e_TSFL1_count,
      e_slot64b_TOP_int_1,
      e_slot64b_TOP_int_0,
      e_map_TSF3_int_1,
      e_map_TSF3_int_0,
      e_map_TSF1_int_1,
      e_map_TSF1_int_0,
      e_N_ST,
      e_map_ST2_int_1,
      e_map_ST2_int_0,
      e_map_ST_int_1,
      e_map_ST_int_0,
      e_map_veto_int_1,
      e_map_veto_int_0,
      e_map_TSF4_int_1,
      e_map_TSF4_int_0,
      e_map_TSF2_int_1,
      e_map_TSF2_int_0,
      e_map_TSF0_int_1,
      e_map_TSF0_int_0,
      e_charge_flag,
      e_charge_sum,
      e_revoclk,
      e_aaaa,
      e_firmid, e_firmver, e_evt, e_clk, e_coml1, e_b2ldly, e_maxrvc
    };

    /** bus bit map. (a downto a-b) **/
    const int BitMap[nLeafs][2] = {

      {2023, 0}, // 2GeV_15
      {2022, 0}, // 2GeV_14
      {2021, 0}, // 2GeV_13
      {2020, 0}, // 2GeV_12
      {2019, 0}, // 2GeV_11
      {2018, 0}, // 2GeV_10
      {2017, 0}, // 2GeV_9
      {2016, 0}, // 2GeV_8
      {2015, 0}, // 2GeV_7
      {2014, 0}, // 2GeV_6
      {2013, 0}, // 2GeV_5
      {2012, 0}, // 2GeV_4
      {2011, 0}, // 2GeV_3
      {2010, 0}, // 2GeV_2
      {2009, 0}, // 2GeV_1
      {2008, 0}, // 2GeV_0
      {2007, 0}, // 1GeV_15
      {2006, 0}, // 1GeV_14
      {2005, 0}, // 1GeV_13
      {2004, 0}, // 1GeV_12
      {2003, 0}, // 1GeV_11
      {2002, 0}, // 1GeV_10
      {2001, 0}, // 1GeV_9
      {2000, 0}, // 1GeV_8
      {1999, 0}, // 1GeV_7
      {1998, 0}, // 1GeV_6
      {1997, 0}, // 1GeV_5
      {1996, 0}, // 1GeV_4
      {1995, 0}, // 1GeV_3
      {1994, 0}, // 1GeV_2
      {1993, 0}, // 1GeV_1
      {1992, 0}, // 1GeV_0

      {1584 + 31, 31}, // wire counter TSF0
      {1552 + 31, 31}, // wire counter TSF2
      {1520 + 31, 31}, // wire counter TSF4
      {1488 + 31, 31}, // wire counter TSF1
      {1456 + 31, 31}, // wire counter TSF3

      {1671 - 216, 6}, // theta_11
      {1664 - 216, 7}, // phi_11
      {1656 - 216, 7}, // t_11
      {1648 - 216, 11}, // E_11

      {1636 - 216, 6}, // theta_10
      {1629 - 216, 7}, // phi_10
      {1621 - 216, 7}, // t_10
      {1613 - 216, 11}, // E_10

      {1601 - 216, 6}, // theta_9
      {1594 - 216, 7}, // phi_9
      {1586 - 216, 7}, // t_9
      {1578 - 216, 11}, // E_9

      {1566 - 216, 6}, // theta_8
      {1559 - 216, 7}, // phi_8
      {1551 - 216, 7}, // t_8
      {1543 - 216, 11}, // E_8

      {1531 - 216, 6}, // theta_7
      {1524 - 216, 7}, // phi_7
      {1516 - 216, 7}, // t_7
      {1508 - 216, 11}, // E_7

      {1496 - 216, 6}, // theta_6
      {1489 - 216, 7}, // phi_6
      {1481 - 216, 7}, // t_6
      {1473 - 216, 11}, // E_6

      {1461 - 216, 1}, // X_1
      {1459 - 216, 2}, // N_cluster_1
      {1456 - 216, 0}, // ECL_trg_1

      {1671 - 216 * 2, 6}, // theta_5
      {1664 - 216 * 2, 7}, // phi_5
      {1656 - 216 * 2, 7}, // t_5
      {1648 - 216 * 2, 11}, // E_5

      {1636 - 216 * 2, 6}, // theta_4
      {1629 - 216 * 2, 7}, // phi_4
      {1621 - 216 * 2, 7}, // t_4
      {1613 - 216 * 2, 11}, // E_4

      {1601 - 216 * 2, 6}, // theta_3
      {1594 - 216 * 2, 7}, // phi_3
      {1586 - 216 * 2, 7}, // t_3
      {1578 - 216 * 2, 11}, // E_3

      {1566 - 216 * 2, 6}, // theta_2
      {1559 - 216 * 2, 7}, // phi_2
      {1551 - 216 * 2, 7}, // t_2
      {1543 - 216 * 2, 11}, // E_2

      {1531 - 216 * 2, 6}, // theta_1
      {1524 - 216 * 2, 7}, // phi_1
      {1516 - 216 * 2, 7}, // t_1
      {1508 - 216 * 2, 11}, // E_1

      {1496 - 216 * 2, 6}, // theta_0
      {1489 - 216 * 2, 7}, // phi_0
      {1481 - 216 * 2, 7}, // t_0
      {1473 - 216 * 2, 11}, // E_0

      {1461 - 216 * 2, 1}, // X_0
      {1459 - 216 * 2, 2}, // N_cluster_0
      {1456 - 216 * 2, 0}, // ECL_trg_0


      {991, 0}, // ECL_trg (991)
      {990, 0}, // ECL_trg_delay (990)
      {998, 2}, // N_track_cut (998 downto 996)
      {995, 2}, // N_track_NN (995 downto 993)
      {992, 2}, // N_track_3D (992 downto 990)
      {989, 9}, // CDCL1_count (989 downto 980)
      {979, 3}, // phi_CDC_int_1 (979 downto 976)
      {975, 31}, // phi_CDC_int_0 (975 downto 943)
      {943, 3}, // phi_i_int_1 (943 downto 940)
      {939, 31}, // phi_i_int_0 (939 downto 908)
      {907, 2}, // N_track (907 downto 905)
      {904, 0}, // b2b_1to3 (904 downto 904)
      {903, 0}, // b2b_1to5 (903 downto 903)
      {902, 0}, // b2b_1to7 (902 downto 902)
      {901, 0}, // b2b_1to9 (901 downto 901)
      {900, 0}, // opening (900 downto 900)
      {899, 0}, // samehem_1 (899)
      {898, 0}, // opphem_1 (898)
      {897, 0}, // c_b2b_1to9_1 (897)
      {896, 0}, // c_b2b_1to7_1 (896)
      {895, 0}, // c_b2b_1to5_1 (895)
      {894, 0}, // c_b2b_1to3_1 (894)
      {893, 0}, // tc_b2b_1to9_1 (893)
      {892, 0}, // tc_b2b_1to7_1 (892)
      {891, 0}, // tc_b2b_1to5_1 (891)
      {890, 0}, // tc_b2b_1to3_1 (890)
      {887, 9}, // TOPL1_count (887 downto 878)
      {877, 9}, // KLML1_count (877 downto 868)
      {867, 7}, // sector_KLM_int (867 downto 860)
      {859, 7}, // sector_CDC_int (859 downto 852)
      {851, 2}, // N_KLMmatch (851 downto 849)
      {848, 0}, // samehem_0 (848)
      {847, 0}, // opphem_0 (847)
      {846, 2}, // N_match_2GeV_0 (846 downto 844)
      {843, 2}, // N_match_0 (843 downto 841)
      {840, 2}, // N_g_1GeV_0 (840 downto 838)
      {837, 2}, // N_g_2GeV_0 (837 downto 835)
      {834, 9}, // ECLL1_count (834 downto 825)
      {824, 0}, // c_b2b_1to9_0 (824)
      {823, 0}, // c_b2b_1to7_0 (823)
      {822, 0}, // c_b2b_1to5_0 (822)
      {821, 0}, // c_b2b_1to3_0 (821)
      {820, 0}, // tc_b2b_1to9_0 (820)
      {819, 0}, // tc_b2b_1to7_0 (819)
      {818, 0}, // tc_b2b_1to5_0 (818)
      {817, 0}, // tc_b2b_1to3_0 (817)
      {816, 9}, // CDC3DL1_count (816 downto 807)
      {806, 9}, // CDCNNL1_count (806 downto 797)
      {796, 15}, // slot_CDC_int
      {780, 15}, // slot_TOP_int
      {764, 2}, // N_TOPmatch
      {761, 2}, // N_match_2GeV_1
      {758, 2}, // N_match_1
      {755, 2}, // N_g_1GeV_1
      {752, 2}, // N_g_2GeV_1
      {749, 9}, // TSFL1_count
      {739, 31}, // slot64b_TOP_int_1
      {708, 31}, // slot64b_TOP_int_0
      {546, 31}, // map_TSF3_int_1 (63 downto 32)
      {514, 31}, // map_TSF3_int_0 (31 downto 0)
      {482, 31}, // map_TSF1_int_1 (63 downto 32)
      {450, 31}, // map_TSF1_int_0 (31 downto 0)
      {418, 2}, // N_ST
      {415, 31}, // map_ST2_int_1 (63 downto 32)
      {383, 31}, // map_ST2_int_0 (31 downto 0)
      {351, 31}, // map_ST_int_1 (63 downto 32)
      {319, 31}, // map_ST_int_0 (31 downto 0)
      {287, 31}, // map_veto_int_1 (63 downto 32)
      {255, 31}, // map_veto_int_0 (31 downto 0)
      {223, 31}, // map_TSF4_int_1 (63 downto 32)
      {191, 31}, // map_TSF4_int_0 (31 downto 0)
      {159, 31}, // map_TSF2_int_1 (63 downto 32)
      {127, 31}, // map_TSF2_int_0 (31 downto 0)
      {95, 31}, // map_TSF0_int_1 (63 downto 32)
      {63, 31}, // map_TSF0_int_0 (31 downto 0)

      {31, 1}, // charge_flag (31 downto 30)
      {29, 2}, // charge_sum (31 downto 30)

      {26, 10}, // revoclk (26 downto 16)
      {15, 15} // aaaa (15 downto 0)
    };

    int
    GetStoreLeaf(TRGGRLUnpackerStore* store, int leafID)
    {
      if (leafID == e_2GeV_15) return store->get_2GeV_ECL(15);
      if (leafID == e_2GeV_14) return store->get_2GeV_ECL(14);
      if (leafID == e_2GeV_13) return store->get_2GeV_ECL(13);
      if (leafID == e_2GeV_12) return store->get_2GeV_ECL(12);
      if (leafID == e_2GeV_11) return store->get_2GeV_ECL(11);
      if (leafID == e_2GeV_10) return store->get_2GeV_ECL(10);
      if (leafID == e_2GeV_9) return store->get_2GeV_ECL(9);
      if (leafID == e_2GeV_8) return store->get_2GeV_ECL(8);
      if (leafID == e_2GeV_7) return store->get_2GeV_ECL(7);
      if (leafID == e_2GeV_6) return store->get_2GeV_ECL(6);
      if (leafID == e_2GeV_5) return store->get_2GeV_ECL(5);
      if (leafID == e_2GeV_4) return store->get_2GeV_ECL(4);
      if (leafID == e_2GeV_3) return store->get_2GeV_ECL(3);
      if (leafID == e_2GeV_2) return store->get_2GeV_ECL(2);
      if (leafID == e_2GeV_1) return store->get_2GeV_ECL(1);
      if (leafID == e_2GeV_0) return store->get_2GeV_ECL(0);
      if (leafID == e_1GeV_15) return store->get_1GeV_ECL(15);
      if (leafID == e_1GeV_14) return store->get_1GeV_ECL(14);
      if (leafID == e_1GeV_13) return store->get_1GeV_ECL(13);
      if (leafID == e_1GeV_12) return store->get_1GeV_ECL(12);
      if (leafID == e_1GeV_11) return store->get_1GeV_ECL(11);
      if (leafID == e_1GeV_10) return store->get_1GeV_ECL(10);
      if (leafID == e_1GeV_9) return store->get_1GeV_ECL(9);
      if (leafID == e_1GeV_8) return store->get_1GeV_ECL(8);
      if (leafID == e_1GeV_7) return store->get_1GeV_ECL(7);
      if (leafID == e_1GeV_6) return store->get_1GeV_ECL(6);
      if (leafID == e_1GeV_5) return store->get_1GeV_ECL(5);
      if (leafID == e_1GeV_4) return store->get_1GeV_ECL(4);
      if (leafID == e_1GeV_3) return store->get_1GeV_ECL(3);
      if (leafID == e_1GeV_2) return store->get_1GeV_ECL(2);
      if (leafID == e_1GeV_1) return store->get_1GeV_ECL(1);
      if (leafID == e_1GeV_0) return store->get_1GeV_ECL(0);

      if (leafID == e_wire_tsf0) return store->get_wire_tsf0();
      if (leafID == e_wire_tsf2) return store->get_wire_tsf2();
      if (leafID == e_wire_tsf4) return store->get_wire_tsf4();
      if (leafID == e_wire_tsf1) return store->get_wire_tsf1();
      if (leafID == e_wire_tsf3) return store->get_wire_tsf3();

      if (leafID == e_theta_11) return store->get_theta_ECL(11);
      if (leafID == e_phi_11) return store->get_phi_ECL(11);
      if (leafID == e_t_11) return store->get_t_ECL(11);
      if (leafID == e_E_11) return store->get_E_ECL(11);
      if (leafID == e_theta_10) return store->get_theta_ECL(10);
      if (leafID == e_phi_10) return store->get_phi_ECL(10);
      if (leafID == e_t_10) return store->get_t_ECL(10);
      if (leafID == e_E_10) return store->get_E_ECL(10);
      if (leafID == e_theta_9) return store->get_theta_ECL(9);
      if (leafID == e_phi_9) return store->get_phi_ECL(9);
      if (leafID == e_t_9) return store->get_t_ECL(9);
      if (leafID == e_E_9) return store->get_E_ECL(9);
      if (leafID == e_theta_8) return store->get_theta_ECL(8);
      if (leafID == e_phi_8) return store->get_phi_ECL(8);
      if (leafID == e_t_8) return store->get_t_ECL(8);
      if (leafID == e_E_8) return store->get_E_ECL(8);
      if (leafID == e_theta_7) return store->get_theta_ECL(7);
      if (leafID == e_phi_7) return store->get_phi_ECL(7);
      if (leafID == e_t_7) return store->get_t_ECL(7);
      if (leafID == e_E_7) return store->get_E_ECL(7);
      if (leafID == e_theta_6) return store->get_theta_ECL(6);
      if (leafID == e_phi_6) return store->get_phi_ECL(6);
      if (leafID == e_t_6) return store->get_t_ECL(6);
      if (leafID == e_E_6) return store->get_E_ECL(6);
      if (leafID == e_X_1) return store->get_X_1();
      if (leafID == e_N_cluster_1) return store->get_N_cluster_1();
      if (leafID == e_ECL_trg_1) return store->get_ECL_trg_1();
      if (leafID == e_theta_5) return store->get_theta_ECL(5);
      if (leafID == e_phi_5) return store->get_phi_ECL(5);
      if (leafID == e_t_5) return store->get_t_ECL(5);
      if (leafID == e_E_5) return store->get_E_ECL(5);
      if (leafID == e_theta_4)  return store->get_theta_ECL(4);
      if (leafID == e_phi_4)  return store->get_phi_ECL(4);
      if (leafID == e_t_4)  return store->get_t_ECL(4);
      if (leafID == e_E_4)  return store->get_E_ECL(4);
      if (leafID == e_theta_3) return store->get_theta_ECL(3);
      if (leafID == e_phi_3) return store->get_phi_ECL(3);
      if (leafID == e_t_3) return store->get_t_ECL(3);
      if (leafID == e_E_3) return store->get_E_ECL(3);
      if (leafID == e_theta_2)  return store->get_theta_ECL(2);
      if (leafID == e_phi_2)  return store->get_phi_ECL(2);
      if (leafID == e_t_2)  return store->get_t_ECL(2);
      if (leafID == e_E_2) return store->get_E_ECL(2);
      if (leafID == e_theta_1) return store->get_theta_ECL(1);
      if (leafID == e_phi_1) return store->get_phi_ECL(1);
      if (leafID == e_t_1) return store->get_t_ECL(1);
      if (leafID == e_E_1) return store->get_E_ECL(1);
      if (leafID == e_theta_0) return store->get_theta_ECL(0);
      if (leafID == e_phi_0) return store->get_phi_ECL(0);
      if (leafID == e_t_0) return store->get_t_ECL(0);
      if (leafID == e_E_0) return store->get_E_ECL(0);
      if (leafID == e_X_0) return store->get_X_0();
      if (leafID == e_N_cluster_0) return store->get_N_cluster_0();
      if (leafID == e_ECL_trg_0) return store->get_ECL_trg_0();
      if (leafID == e_ECL_trg) return store->get_ECL_trg();
      if (leafID == e_ECL_trg_delay) return store->get_ECL_trg_delay();
      if (leafID == e_N_track_cut) return store->get_N_track_cut();
      if (leafID == e_N_track_NN) return store->get_N_track_NN();
      if (leafID == e_N_track_3D) return store->get_N_track_3D();
      if (leafID == e_CDCL1_count) return store->get_CDCL1_count();
      if (leafID == e_phi_CDC_int_1) return store->get_phi_CDC_int(1);
      if (leafID == e_phi_CDC_int_0) return store->get_phi_CDC_int(0);
      if (leafID == e_phi_i_int_1) return store->get_phi_i_int(1);
      if (leafID == e_phi_i_int_0) return store->get_phi_i_int(0);
      if (leafID == e_N_track) return store->get_N_track();
      if (leafID == e_b2b_1to3) return store->get_b2b_1to3();
      if (leafID == e_b2b_1to5) return store->get_b2b_1to5();
      if (leafID == e_b2b_1to7) return store->get_b2b_1to7();
      if (leafID == e_b2b_1to9) return store->get_b2b_1to9();
      if (leafID == e_opening) return store->get_opening();
      if (leafID == e_samehem_1) return store->get_samehem(1);
      if (leafID == e_opphem_1) return store->get_opphem(1);
      if (leafID == e_c_b2b_1to9_1) return store->get_c_b2b_1to9(1);
      if (leafID == e_c_b2b_1to7_1) return store->get_c_b2b_1to7(1);
      if (leafID == e_c_b2b_1to5_1) return store->get_c_b2b_1to5(1);
      if (leafID == e_c_b2b_1to3_1) return store->get_c_b2b_1to3(1);
      if (leafID == e_tc_b2b_1to9_1) return store->get_tc_b2b_1to9(1);
      if (leafID == e_tc_b2b_1to7_1) return store->get_tc_b2b_1to7(1);
      if (leafID == e_tc_b2b_1to5_1) return store->get_tc_b2b_1to5(1);
      if (leafID == e_tc_b2b_1to3_1) return store->get_tc_b2b_1to3(1);
      if (leafID == e_TOPL1_count) return store->get_TOPL1_count();
      if (leafID == e_KLML1_count) return store->get_KLML1_count();
      if (leafID == e_sector_KLM_int) return store->get_sector_KLM_int();
      if (leafID == e_sector_CDC_int) return store->get_sector_CDC_int();
      if (leafID == e_N_KLMmatch) return store->get_N_KLMmatch();
      if (leafID == e_samehem_0) return store->get_samehem(0);
      if (leafID == e_opphem_0) return store->get_opphem(0);
      if (leafID == e_N_match_2GeV_0) return store->get_N_match_2GeV(0);
      if (leafID == e_N_match_0) return store->get_N_match(0);
      if (leafID == e_N_g_1GeV_0) return store->get_N_g_1GeV(0);
      if (leafID == e_N_g_2GeV_0) return store->get_N_g_2GeV(0);
      if (leafID == e_ECLL1_count) return store->get_ECLL1_count();
      if (leafID == e_c_b2b_1to9_0) return store->get_c_b2b_1to9(0);
      if (leafID == e_c_b2b_1to7_0) return store->get_c_b2b_1to7(0);
      if (leafID == e_c_b2b_1to5_0) return store->get_c_b2b_1to5(0);
      if (leafID == e_c_b2b_1to3_0) return store->get_c_b2b_1to3(0);
      if (leafID == e_tc_b2b_1to9_0) return store->get_tc_b2b_1to9(0);
      if (leafID == e_tc_b2b_1to7_0) return store->get_tc_b2b_1to7(0);
      if (leafID == e_tc_b2b_1to5_0) return store->get_tc_b2b_1to5(0);
      if (leafID == e_tc_b2b_1to3_0) return store->get_tc_b2b_1to3(0);
      if (leafID == e_CDC3DL1_count) return store->get_CDC3DL1_count();
      if (leafID == e_CDCNNL1_count) return store->get_CDCNNL1_count();
      if (leafID == e_slot_CDC_int) return store->get_slot_CDC_int();
      if (leafID == e_slot_TOP_int) return store->get_slot_TOP_int();
      if (leafID == e_N_TOPmatch) return store->get_N_TOPmatch();
      if (leafID == e_N_match_2GeV_1) return store->get_N_match_2GeV(1);
      if (leafID == e_N_match_1) return store->get_N_match(1);
      if (leafID == e_N_g_1GeV_1) return store->get_N_g_1GeV(1);
      if (leafID == e_N_g_2GeV_1) return store->get_N_g_2GeV(1);
      if (leafID == e_TSFL1_count) return store->get_TSFL1_count();
      if (leafID == e_slot64b_TOP_int_1) return store->get_slot64b_TOP_int_1();
      if (leafID == e_slot64b_TOP_int_0) return store->get_slot64b_TOP_int_0();
      if (leafID == e_map_TSF3_int_1) return store->get_map_TSF3_int(1);
      if (leafID == e_map_TSF3_int_0) return store->get_map_TSF3_int(0);
      if (leafID == e_map_TSF1_int_1) return store->get_map_TSF1_int(1);
      if (leafID == e_map_TSF1_int_0) return store->get_map_TSF1_int(0);
      if (leafID == e_N_ST) return store->get_N_ST();
      if (leafID == e_map_ST2_int_1) return store->get_map_ST2_int(1);
      if (leafID == e_map_ST2_int_0) return store->get_map_ST2_int(0);
      if (leafID == e_map_ST_int_1) return store->get_map_ST_int(1);
      if (leafID == e_map_ST_int_0) return store->get_map_ST_int(0);
      if (leafID == e_map_veto_int_1) return store->get_map_veto_int(1);
      if (leafID == e_map_veto_int_0) return store->get_map_veto_int(0);
      if (leafID == e_map_TSF4_int_1) return store->get_map_TSF4_int(1);
      if (leafID == e_map_TSF4_int_0) return store->get_map_TSF4_int(0);
      if (leafID == e_map_TSF2_int_1) return store->get_map_TSF2_int(1);
      if (leafID == e_map_TSF2_int_0) return store->get_map_TSF2_int(0);
      if (leafID == e_map_TSF0_int_1) return store->get_map_TSF0_int(1);
      if (leafID == e_map_TSF0_int_0) return store->get_map_TSF0_int(0);
      if (leafID == e_charge_flag) return store->get_charge_flag();
      if (leafID == e_charge_sum) return store->get_charge_sum();
      if (leafID == e_revoclk) return store->get_revoclk();
      if (leafID == e_aaaa) return store->get_aaaa();
      if (leafID == e_evt) return store->get_evt();
      if (leafID == e_clk) return store->get_clk();
      if (leafID == e_firmid) return store->get_firmid();
      if (leafID == e_firmver) return store->get_firmver();
      if (leafID == e_coml1) return store->get_coml1();
      if (leafID == e_b2ldly) return store->get_b2ldly();
      if (leafID == e_maxrvc) return store->get_maxrvc();
      else {
        B2WARNING("!!!getting leaf out of list!!!");
        return 0;
      }
    };

    void
    SetStoreLeaf(TRGGRLUnpackerStore* store, int leafID, int value)
    {
      if (leafID == e_2GeV_15) store->set_2GeV_ECL(15, value);
      if (leafID == e_2GeV_14) store->set_2GeV_ECL(14, value);
      if (leafID == e_2GeV_13) store->set_2GeV_ECL(13, value);
      if (leafID == e_2GeV_12) store->set_2GeV_ECL(12, value);
      if (leafID == e_2GeV_11) store->set_2GeV_ECL(11, value);
      if (leafID == e_2GeV_10) store->set_2GeV_ECL(10, value);
      if (leafID == e_2GeV_9) store->set_2GeV_ECL(9, value);
      if (leafID == e_2GeV_8) store->set_2GeV_ECL(8, value);
      if (leafID == e_2GeV_7) store->set_2GeV_ECL(7, value);
      if (leafID == e_2GeV_6) store->set_2GeV_ECL(6, value);
      if (leafID == e_2GeV_5) store->set_2GeV_ECL(5, value);
      if (leafID == e_2GeV_4) store->set_2GeV_ECL(4, value);
      if (leafID == e_2GeV_3) store->set_2GeV_ECL(3, value);
      if (leafID == e_2GeV_2) store->set_2GeV_ECL(2, value);
      if (leafID == e_2GeV_1) store->set_2GeV_ECL(1, value);
      if (leafID == e_2GeV_0) store->set_2GeV_ECL(0, value);
      if (leafID == e_1GeV_15) store->set_1GeV_ECL(15, value);
      if (leafID == e_1GeV_14) store->set_1GeV_ECL(14, value);
      if (leafID == e_1GeV_13) store->set_1GeV_ECL(13, value);
      if (leafID == e_1GeV_12) store->set_1GeV_ECL(12, value);
      if (leafID == e_1GeV_11) store->set_1GeV_ECL(11, value);
      if (leafID == e_1GeV_10) store->set_1GeV_ECL(10, value);
      if (leafID == e_1GeV_9) store->set_1GeV_ECL(9, value);
      if (leafID == e_1GeV_8) store->set_1GeV_ECL(8, value);
      if (leafID == e_1GeV_7) store->set_1GeV_ECL(7, value);
      if (leafID == e_1GeV_6) store->set_1GeV_ECL(6, value);
      if (leafID == e_1GeV_5) store->set_1GeV_ECL(5, value);
      if (leafID == e_1GeV_4) store->set_1GeV_ECL(4, value);
      if (leafID == e_1GeV_3) store->set_1GeV_ECL(3, value);
      if (leafID == e_1GeV_2) store->set_1GeV_ECL(2, value);
      if (leafID == e_1GeV_1) store->set_1GeV_ECL(1, value);
      if (leafID == e_1GeV_0) store->set_1GeV_ECL(0, value);

      if (leafID == e_wire_tsf0) store->set_wire_tsf0(value);
      if (leafID == e_wire_tsf2) store->set_wire_tsf2(value);
      if (leafID == e_wire_tsf4) store->set_wire_tsf4(value);
      if (leafID == e_wire_tsf1) store->set_wire_tsf1(value);
      if (leafID == e_wire_tsf3) store->set_wire_tsf3(value);

      if (leafID == e_theta_11) store->set_theta_ECL(11, value);
      if (leafID == e_phi_11) store->set_phi_ECL(11, value);
      if (leafID == e_t_11) store->set_t_ECL(11, value);
      if (leafID == e_E_11) store->set_E_ECL(11, value);
      if (leafID == e_theta_10) store->set_theta_ECL(10, value);
      if (leafID == e_phi_10) store->set_phi_ECL(10, value);
      if (leafID == e_t_10) store->set_t_ECL(10, value);
      if (leafID == e_E_10) store->set_E_ECL(10, value);
      if (leafID == e_theta_9) store->set_theta_ECL(9, value);
      if (leafID == e_phi_9) store->set_phi_ECL(9, value);
      if (leafID == e_t_9) store->set_t_ECL(9, value);
      if (leafID == e_E_9) store->set_E_ECL(9, value);
      if (leafID == e_theta_8) store->set_theta_ECL(8, value);
      if (leafID == e_phi_8) store->set_phi_ECL(8, value);
      if (leafID == e_t_8) store->set_t_ECL(8, value);
      if (leafID == e_E_8) store->set_E_ECL(8, value);
      if (leafID == e_theta_7) store->set_theta_ECL(7, value);
      if (leafID == e_phi_7) store->set_phi_ECL(7, value);
      if (leafID == e_t_7) store->set_t_ECL(7, value);
      if (leafID == e_E_7) store->set_E_ECL(7, value);
      if (leafID == e_theta_6) store->set_theta_ECL(6, value);
      if (leafID == e_phi_6) store->set_phi_ECL(6, value);
      if (leafID == e_t_6) store->set_t_ECL(6, value);
      if (leafID == e_E_6) store->set_E_ECL(6, value);
      if (leafID == e_X_1) store->set_X_1(value);
      if (leafID == e_N_cluster_1) store->set_N_cluster_1(value);
      if (leafID == e_ECL_trg_1) store->set_ECL_trg_1(value);
      if (leafID == e_theta_5) store->set_theta_ECL(5, value);
      if (leafID == e_phi_5) store->set_phi_ECL(5, value);
      if (leafID == e_t_5) store->set_t_ECL(5, value);
      if (leafID == e_E_5) store->set_E_ECL(5, value);
      if (leafID == e_theta_4)  store->set_theta_ECL(4, value);
      if (leafID == e_phi_4)  store->set_phi_ECL(4, value);
      if (leafID == e_t_4)  store->set_t_ECL(4, value);
      if (leafID == e_E_4)  store->set_E_ECL(4, value);
      if (leafID == e_theta_3) store->set_theta_ECL(3, value);
      if (leafID == e_phi_3) store->set_phi_ECL(3, value);
      if (leafID == e_t_3) store->set_t_ECL(3, value);
      if (leafID == e_E_3) store->set_E_ECL(3, value);
      if (leafID == e_theta_2)  store->set_theta_ECL(2, value);
      if (leafID == e_phi_2)  store->set_phi_ECL(2, value);
      if (leafID == e_t_2)  store->set_t_ECL(2, value);
      if (leafID == e_E_2) store->set_E_ECL(2, value);
      if (leafID == e_theta_1) store->set_theta_ECL(1, value);
      if (leafID == e_phi_1) store->set_phi_ECL(1, value);
      if (leafID == e_t_1) store->set_t_ECL(1, value);
      if (leafID == e_E_1) store->set_E_ECL(1, value);
      if (leafID == e_theta_0) store->set_theta_ECL(0, value);
      if (leafID == e_phi_0) store->set_phi_ECL(0, value);
      if (leafID == e_t_0) store->set_t_ECL(0, value);
      if (leafID == e_E_0) store->set_E_ECL(0, value);
      if (leafID == e_X_0) store->set_X_0(value);
      if (leafID == e_N_cluster_0) store->set_N_cluster_0(value);
      if (leafID == e_ECL_trg_0) store->set_ECL_trg_0(value);
      if (leafID == e_ECL_trg) store->set_ECL_trg(value);
      if (leafID == e_ECL_trg_delay) store->set_ECL_trg_delay(value);
      if (leafID == e_N_track_cut) store->set_N_track_cut(value);
      if (leafID == e_N_track_NN) store->set_N_track_NN(value);
      if (leafID == e_N_track_3D) store->set_N_track_3D(value);
      if (leafID == e_CDCL1_count) store->set_CDCL1_count(value);
      if (leafID == e_phi_CDC_int_1) store->set_phi_CDC_int(1, value);
      if (leafID == e_phi_CDC_int_0) store->set_phi_CDC_int(0, value);
      if (leafID == e_phi_i_int_1) store->set_phi_i_int(1, value);
      if (leafID == e_phi_i_int_0) store->set_phi_i_int(0, value);
      if (leafID == e_N_track) store->set_N_track(value);
      if (leafID == e_b2b_1to3) store->set_b2b_1to3(value);
      if (leafID == e_b2b_1to5) store->set_b2b_1to5(value);
      if (leafID == e_b2b_1to7) store->set_b2b_1to7(value);
      if (leafID == e_b2b_1to9) store->set_b2b_1to9(value);
      if (leafID == e_opening) store->set_opening(value);
      if (leafID == e_samehem_1) store->set_samehem(1, value);
      if (leafID == e_opphem_1) store->set_opphem(1, value);
      if (leafID == e_c_b2b_1to9_1) store->set_c_b2b_1to9(1, value);
      if (leafID == e_c_b2b_1to7_1) store->set_c_b2b_1to7(1, value);
      if (leafID == e_c_b2b_1to5_1) store->set_c_b2b_1to5(1, value);
      if (leafID == e_c_b2b_1to3_1) store->set_c_b2b_1to3(1, value);
      if (leafID == e_tc_b2b_1to9_1) store->set_tc_b2b_1to9(1, value);
      if (leafID == e_tc_b2b_1to7_1) store->set_tc_b2b_1to7(1, value);
      if (leafID == e_tc_b2b_1to5_1) store->set_tc_b2b_1to5(1, value);
      if (leafID == e_tc_b2b_1to3_1) store->set_tc_b2b_1to3(1, value);
      if (leafID == e_TOPL1_count) store->set_TOPL1_count(value);
      if (leafID == e_KLML1_count) store->set_KLML1_count(value);
      if (leafID == e_sector_KLM_int) store->set_sector_KLM_int(value);
      if (leafID == e_sector_CDC_int) store->set_sector_CDC_int(value);
      if (leafID == e_N_KLMmatch) store->set_N_KLMmatch(value);
      if (leafID == e_samehem_0) store->set_samehem(0, value);
      if (leafID == e_opphem_0) store->set_opphem(0, value);
      if (leafID == e_N_match_2GeV_0) store->set_N_match_2GeV(0, value);
      if (leafID == e_N_match_0) store->set_N_match(0, value);
      if (leafID == e_N_g_1GeV_0) store->set_N_g_1GeV(0, value);
      if (leafID == e_N_g_2GeV_0) store->set_N_g_2GeV(0, value);
      if (leafID == e_ECLL1_count) store->set_ECLL1_count(value);
      if (leafID == e_c_b2b_1to9_0) store->set_c_b2b_1to9(0, value);
      if (leafID == e_c_b2b_1to7_0) store->set_c_b2b_1to7(0, value);
      if (leafID == e_c_b2b_1to5_0) store->set_c_b2b_1to5(0, value);
      if (leafID == e_c_b2b_1to3_0) store->set_c_b2b_1to3(0, value);
      if (leafID == e_tc_b2b_1to9_0) store->set_tc_b2b_1to9(0, value);
      if (leafID == e_tc_b2b_1to7_0) store->set_tc_b2b_1to7(0, value);
      if (leafID == e_tc_b2b_1to5_0) store->set_tc_b2b_1to5(0, value);
      if (leafID == e_tc_b2b_1to3_0) store->set_tc_b2b_1to3(0, value);
      if (leafID == e_CDC3DL1_count) store->set_CDC3DL1_count(value);
      if (leafID == e_CDCNNL1_count) store->set_CDCNNL1_count(value);
      if (leafID == e_slot_CDC_int) store->set_slot_CDC_int(value);
      if (leafID == e_slot_TOP_int) store->set_slot_TOP_int(value);
      if (leafID == e_N_TOPmatch) store->set_N_TOPmatch(value);
      if (leafID == e_N_match_2GeV_1) store->set_N_match_2GeV(1, value);
      if (leafID == e_N_match_1) store->set_N_match(1, value);
      if (leafID == e_N_g_1GeV_1) store->set_N_g_1GeV(1, value);
      if (leafID == e_N_g_2GeV_1) store->set_N_g_2GeV(1, value);
      if (leafID == e_TSFL1_count) store->set_TSFL1_count(value);
      if (leafID == e_slot64b_TOP_int_1) store->set_slot64b_TOP_int_1(value);
      if (leafID == e_slot64b_TOP_int_0) store->set_slot64b_TOP_int_0(value);
      if (leafID == e_map_TSF3_int_1) store->set_map_TSF3_int(1, value);
      if (leafID == e_map_TSF3_int_0) store->set_map_TSF3_int(0, value);
      if (leafID == e_map_TSF1_int_1) store->set_map_TSF1_int(1, value);
      if (leafID == e_map_TSF1_int_0) store->set_map_TSF1_int(0, value);
      if (leafID == e_N_ST) store->set_N_ST(value);
      if (leafID == e_map_ST2_int_1) store->set_map_ST2_int(1, value);
      if (leafID == e_map_ST2_int_0) store->set_map_ST2_int(0, value);
      if (leafID == e_map_ST_int_1) store->set_map_ST_int(1, value);
      if (leafID == e_map_ST_int_0) store->set_map_ST_int(0, value);
      if (leafID == e_map_veto_int_1) store->set_map_veto_int(1, value);
      if (leafID == e_map_veto_int_0) store->set_map_veto_int(0, value);
      if (leafID == e_map_TSF4_int_1) store->set_map_TSF4_int(1, value);
      if (leafID == e_map_TSF4_int_0) store->set_map_TSF4_int(0, value);
      if (leafID == e_map_TSF2_int_1) store->set_map_TSF2_int(1, value);
      if (leafID == e_map_TSF2_int_0) store->set_map_TSF2_int(0, value);
      if (leafID == e_map_TSF0_int_1) store->set_map_TSF0_int(1, value);
      if (leafID == e_map_TSF0_int_0) store->set_map_TSF0_int(0, value);
      if (leafID == e_charge_flag) store->set_charge_flag(value);
      if (leafID == e_charge_sum) store->set_charge_sum(value);
      if (leafID == e_revoclk) store->set_revoclk(value);
      if (leafID == e_aaaa) store->set_aaaa(value);
      if (leafID == e_evt) store->set_evt(value);
      if (leafID == e_clk) store->set_clk(value);
      if (leafID == e_firmid) store->set_firmid(value);
      if (leafID == e_firmver) store->set_firmver(value);
      if (leafID == e_coml1) store->set_coml1(value);
      if (leafID == e_b2ldly) store->set_b2ldly(value);
      if (leafID == e_maxrvc) store->set_maxrvc(value);
    };

    /*! A module of TRGGRLUnpacker */
    class TRGGRLUnpackerModule : public Module {

    public:

      /** Constructor */
      TRGGRLUnpackerModule();

      /** Destructor */
      virtual ~TRGGRLUnpackerModule();

      /** Initilizes TRGGRLUnpackerUnpackerModule.*/
      virtual void initialize() override;

      /** Called event by event.*/
      virtual void event() override;

      /** Called when processing ended.*/
      virtual void terminate() override;

      /** Called when new run started.*/
      virtual void beginRun() override;

      /** Called when run ended*/
      virtual void endRun() override;

      /** returns version of TRGGRLUnpackerModule.*/
      std::string version() const;

      /** Unpacker main function.*/
      virtual void fillTreeTRGGRLUnpacker(int* buf, int evt);

    private:

      /** PCIe40 data or copper data **/
      bool m_pciedata;

    };
  }
} // end namespace Belle2
#endif // TRGGRLUNPACKER_H
