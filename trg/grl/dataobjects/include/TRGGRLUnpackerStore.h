/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGGRLUnpackerStore.h
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@post.kek.jp
//-----------------------------------------------------------
// Description : A dataobject for TRGGRLUnpacker.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGGRLUNPACKERSTORE_H
#define TRGGRLUNPACKERSTORE_H

#include <TObject.h>
#include <vector>

namespace Belle2 {

  //! Example Detector
  class TRGGRLUnpackerStore : public TObject {
  public:

    /**m_dddd leaf*/
    int m_dddd;
    /**m_unamed leaf*/
    int m_unamed;
    /**m_revoclk leaf*/
    int m_revoclk;
    /**m_ECL_trg leaf*/
    int m_ECL_trg;
    /**m_ECL_trg_delay leaf*/
    int m_ECL_trg_delay;
    /**m_CDCECL_jitter leaf*/
    int m_CDCECL_jitter;
    /**m_phi_CDC integer leaf*/
    int m_phi_CDC_int[2];
    /**m_phi_CDC map leaf*/
    std::vector<bool> m_phi_CDC = std::vector<bool>(36, 0);
    /**m_phi_i integer leaf*/
    int m_phi_i_int[2];
    /**m_phi_i map leaf*/
    std::vector<bool> m_phi_i = std::vector<bool>(36, 0);
    /**m_N_track leaf*/
    int m_N_track;
    /**m_N_track_cut leaf*/
    int m_N_track_cut;
    /**m_N_track_3D leaf*/
    int m_N_track_3D;
    /**m_N_track_NN leaf*/
    int m_N_track_NN;
    /**m_N_ST leaf*/
    int m_N_ST;
    /**m_b2b_1to3 leaf*/
    int m_b2b_1to3;
    /**m_b2b_1to5 leaf*/
    int m_b2b_1to5;
    /**m_b2b_1to7 leaf*/
    int m_b2b_1to7;
    /**m_b2b_1to9 leaf*/
    int m_b2b_1to9;
    /**m_opening leaf*/
    int m_opening;
    /**m_cc leaf*/
    int m_cc;
    /**m_unamed0 leaf*/
    int m_unamed0;
    /**m_KLM_trg leaf*/
    int m_KLM_trg;
    /**m_matched_2DKLM leaf*/
    int m_matched_2DKLM;
    /**m_CDCKLM_jitter leaf*/
    int m_CDCKLM_jitter;
    /**m_KLMCDC_jitter leaf*/
    int m_KLMCDC_jitter;
    /**m_sector_KLM integer leaf*/
    int m_sector_KLM_int;
    /**m_sector_KLM map leaf*/
    std::vector<bool> m_sector_KLM = std::vector<bool>(8, 0);
    /**m_sector_CDC integer leaf*/
    int m_sector_CDC_int;
    /**m_sector_CDC map */
    std::vector<bool> m_sector_CDC = std::vector<bool>(8, 0);
    /**m_N_KLMmatch leaf*/
    int m_N_KLMmatch;
    /**m_samehem leaf*/
    std::vector<int> m_samehem = std::vector<int>(2, 0);
    /**m_opphem leaf*/
    std::vector<int> m_opphem = std::vector<int>(2, 0);
    /**m_N_match_2GeV leaf*/
    std::vector<int> m_N_match_2GeV = std::vector<int>(2, 0);
    /**m_N_match leaf*/
    std::vector<int> m_N_match = std::vector<int>(2, 0);
    /**m_N_g_1GeV leaf*/
    std::vector<int> m_N_g_1GeV = std::vector<int>(2, 0);
    /**m_N_g_2GeV leaf*/
    std::vector<int> m_N_g_2GeV = std::vector<int>(2, 0);
    /**m_CDCTOP_jitter leaf*/
    int m_CDCTOP_jitter;
    /**m_c_b2b_1to9 leaf*/
    std::vector<int> m_c_b2b_1to9 = std::vector<int>(2, 0);
    /**m_c_b2b_1to7 leaf*/
    std::vector<int> m_c_b2b_1to7 = std::vector<int>(2, 0);
    /**m_c_b2b_1to5 leaf*/
    std::vector<int> m_c_b2b_1to5 = std::vector<int>(2, 0);
    /**m_c_b2b_1to3 leaf*/
    std::vector<int> m_c_b2b_1to3 = std::vector<int>(2, 0);
    /**m_tc_b2b_1to9 leaf*/
    std::vector<int> m_tc_b2b_1to9 = std::vector<int>(2, 0);
    /**m_tc_b2b_1to7 leaf*/
    std::vector<int> m_tc_b2b_1to7 = std::vector<int>(2, 0);
    /**m_tc_b2b_1to5 leaf*/
    std::vector<int> m_tc_b2b_1to5 = std::vector<int>(2, 0);
    /**m_tc_b2b_1to3 leaf*/
    std::vector<int> m_tc_b2b_1to3 = std::vector<int>(2, 0);
    /**m_CDC3D_jitter leaf*/
    int m_CDC3D_jitter;
    /**m_CDCNN_jitter leaf*/
    int m_CDCNN_jitter;

    /**m_slot_CDC integer leaf*/
    int m_slot_CDC_int;
    /**m_slot_CDC map leaf*/
    std::vector<bool> m_slot_CDC = std::vector<bool>(15, 0);
    /**m_slot_TOP integer leaf*/
    int m_slot_TOP_int;
    /**m_slot_TOP map leaf*/
    std::vector<bool> m_slot_TOP = std::vector<bool>(15, 0);
    /**m_N_TOPmatch leaf*/
    int m_N_TOPmatch;

    /**m_unamed1 leaf*/
    int m_unamed1;
    /**m_unamed2 leaf*/
    int m_unamed2;
    /**m_coincidence_2DECL leaf*/
    int m_coincidence_2DECL;
    /**m_matched_2DECL leaf*/
    int m_matched_2DECL;

    /**m_theta_ECL leaf*/
    std::vector<int> m_theta_ECL = std::vector<int>(30, 0);
    /**m_phi_ECL leaf*/
    std::vector<int> m_phi_ECL = std::vector<int>(30, 0);
    /**m_t_ECL leaf*/
    std::vector<int> m_t_ECL = std::vector<int>(30, 0);
    /**m_E_ECL leaf*/
    std::vector<int> m_E_ECL = std::vector<int>(30, 0);
    /**m_clk_ECL leaf*/
    std::vector<int> m_clk_ECL = std::vector<int>(30, 0);
    /**m_1GeV_flag_ECL leaf*/
    std::vector<int> m_1GeV_ECL = std::vector<int>(30, 0);
    /**m_2GeV_flag_ECL leaf*/
    std::vector<int> m_2GeV_ECL = std::vector<int>(30, 0);

    /**m_map_TSF0 integer leaf*/
    int m_map_TSF0_int[2];
    /**m_map_TSF2 integer leaf*/
    int m_map_TSF2_int[2];
    /**m_map_TSF4 integer leaf*/
    int m_map_TSF4_int[2];
    /**m_map_TSF1 integer leaf*/
    int m_map_TSF1_int[2];
    /**m_map_TSF3 integer leaf*/
    int m_map_TSF3_int[2];
    /**m_map_veto integer leaf*/
    int m_map_veto_int[2];
    /**m_map_ST integer leaf*/
    int m_map_ST_int[2];
    /**m_map_ST2 integer leaf*/
    int m_map_ST2_int[2];
    /**m_map_TSF0 map leaf*/
    std::vector<bool> m_map_TSF0 = std::vector<bool>(64, 0);
    /**m_map_TSF2 map leaf*/
    std::vector<bool> m_map_TSF2 = std::vector<bool>(64, 0);
    /**m_map_TSF4 map leaf*/
    std::vector<bool> m_map_TSF4 = std::vector<bool>(64, 0);
    /**m_map_TSF1 map leaf*/
    std::vector<bool> m_map_TSF1 = std::vector<bool>(64, 0);
    /**m_map_TSF3 map leaf*/
    std::vector<bool> m_map_TSF3 = std::vector<bool>(64, 0);
    /**m_map_veto map leaf*/
    std::vector<bool> m_map_veto = std::vector<bool>(64, 0);
    /**m_map_ST map leaf*/
    std::vector<bool> m_map_ST = std::vector<bool>(64, 0);
    /**m_map_ST2 map leaf*/
    std::vector<bool> m_map_ST2 = std::vector<bool>(64, 0);

    /**m_X leaf*/
    int m_X;
    /**m_X_0 leaf*/
    int m_X_0;
    /**m_X_1 leaf*/
    int m_X_1;
    /**m_X_2 leaf*/
    int m_X_2;
    /**m_N_cluster leaf*/
    int m_N_cluster;
    /**m_N_cluster_0 leaf*/
    int m_N_cluster_0;
    /**m_N_cluster_1 leaf*/
    int m_N_cluster_1;
    /**m_N_cluster_2 leaf*/
    int m_N_cluster_2;
    /**m_revo_fam leaf*/
    int m_revo_fam;
    /**m_ECL_timing leaf*/
    int m_ECL_timing;
    /**m_ECL_trg_0 leaf*/
    int m_ECL_trg_0;
    /**m_ECL_trg_1 leaf*/
    int m_ECL_trg_1;
    /**m_ECL_trg_2 leaf*/
    int m_ECL_trg_2;
    /**m_TOP_trg leaf*/
    int m_TOP_trg;
    /**m_TSFL1_count leaf*/
    int m_TSFL1_count;
    /**m_slot64b_TOP_int_1 leaf*/
    int m_slot64b_TOP_int_1;
    /**m_slot64b_TOP_int_0 leaf*/
    int m_slot64b_TOP_int_0;
    /**m_ECLL1_count leaf*/
    int m_ECLL1_count;
    /**m_CDCL1_count leaf*/
    int m_CDCL1_count;
    /**m_TOPL1_count leaf*/
    int m_TOPL1_count;
    /**m_KLML1_count leaf*/
    int m_KLML1_count;
    /**m_CDC3DL1_count leaf*/
    int m_CDC3DL1_count;
    /**m_CDCNNL1_count leaf*/
    int m_CDCNNL1_count;
    /**m_unamed3 leaf*/
    int m_unamed3;
    /**m_revoclk1 leaf*/
    int m_revoclk1;
    /**m_aaaa leaf*/
    int m_aaaa;
    /**m_charge_flag leaf*/
    int m_charge_flag;
    /**m_charge_sum leaf*/
    int m_charge_sum;
    /**m_firmid leafs*/
    int m_firmid;
    /**m_firmver leafs*/
    int m_firmver;
    /**m_evt leafs*/
    int m_evt;
    /**m_clk leafs*/
    int m_clk;
    /**m_coml1 leafs*/
    int m_coml1;
    /**m_b2ldly leafs*/
    int m_b2ldly;
    /**m_maxrvc leafs*/
    int m_maxrvc;
    /** the class title*/
    ClassDef(TRGGRLUnpackerStore, 1);

  };

} // end namespace Belle2

#endif
