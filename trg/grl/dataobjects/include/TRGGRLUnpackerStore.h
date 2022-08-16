/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGGRLUNPACKERSTORE_H
#define TRGGRLUNPACKERSTORE_H

#include <TObject.h>
#include <vector>

namespace Belle2 {

  //! Example Detector
  class TRGGRLUnpackerStore : public TObject {
  public:
    /**m_dddd leaf*/
    int get_dddd() const {return m_dddd;}
    /**m_unamed leaf*/
    int get_unamed() const {return m_unamed;}
    /**m_revoclk leaf*/
    int get_revoclk() const {return m_revoclk;}
    /**m_ECL_trg leaf*/
    int get_ECL_trg() const {return m_ECL_trg;}
    /**m_ECL_trg_delay leaf*/
    int get_ECL_trg_delay() const {return m_ECL_trg_delay;}
    /**m_CDCECL_jitter leaf*/
    int get_CDCECL_jitter() const {return m_CDCECL_jitter;}
    /**m_phi_CDC integer leaf*/
    int get_phi_CDC_int(int i) const {return  m_phi_CDC_int[i];}
    /**m_phi_CDC map leaf*/
    bool get_phi_CDC(int i) const {return m_phi_CDC[i];}
    /**m_phi_i integer leaf*/
    int get_phi_i_int(int i) const {return m_phi_i_int[i];}
    /**m_phi_i map leaf*/
    bool get_phi_i(int i) const {return m_phi_i[i];}
    /**m_N_track leaf*/
    int get_N_track() const {return m_N_track;}
    /**m_N_track_cut leaf*/
    int get_N_track_cut() const {return m_N_track_cut;}
    /**m_N_track_3D leaf*/
    int get_N_track_3D() const {return m_N_track_3D;}
    /**m_N_track_NN leaf*/
    int get_N_track_NN() const {return m_N_track_NN;}
    /**m_N_ST leaf*/
    int get_N_ST() const {return m_N_ST;}
    /**m_b2b_1to3 leaf*/
    int get_b2b_1to3() const {return m_b2b_1to3;}
    /**m_b2b_1to5 leaf*/
    int get_b2b_1to5() const {return m_b2b_1to5;}
    /**m_b2b_1to7 leaf*/
    int get_b2b_1to7() const {return m_b2b_1to7;}
    /**m_b2b_1to9 leaf*/
    int get_b2b_1to9() const {return m_b2b_1to9;}
    /**m_opening leaf*/
    int get_opening() const {return m_opening;}
    /**m_cc leaf*/
    int get_cc() const {return m_cc;}
    /**m_unamed0 leaf*/
    int get_unamed0() const {return m_unamed0;}
    /**m_KLM_trg leaf*/
    int get_KLM_trg() const {return m_KLM_trg;}
    /**m_matched_2DKLM leaf*/
    int get_matched_2DKLM() const {return m_matched_2DKLM;}
    /**m_CDCKLM_jitter leaf*/
    int get_CDCKLM_jitter() const {return m_CDCKLM_jitter;}
    /**m_KLMCDC_jitter leaf*/
    int get_KLMCDC_jitter() const {return m_KLMCDC_jitter;}
    /**m_sector_KLM integer leaf*/
    int get_sector_KLM_int() const {return m_sector_KLM_int;}
    /**m_sector_KLM map leaf*/
    bool get_sector_KLM(int i) const {return m_sector_KLM[i];}
    /**m_sector_CDC integer leaf*/
    int get_sector_CDC_int() const {return m_sector_CDC_int;}
    /**m_sector_CDC map */
    bool get_sector_CDC(int i) const {return m_sector_CDC[i];}
    /**m_N_KLMmatch leaf*/
    int get_N_KLMmatch() const {return m_N_KLMmatch;}
    /**m_samehem leaf*/
    int get_samehem(int i) const {return  m_samehem[i];}
    /**m_opphem leaf*/
    int get_opphem(int i) const {return m_opphem[i];}
    /**m_N_match_2GeV leaf*/
    int get_N_match_2GeV(int i) const {return m_N_match_2GeV[i];}
    /**m_N_match leaf*/
    int get_N_match(int i) const {return  m_N_match[i];}
    /**m_N_g_1GeV leaf*/
    int get_N_g_1GeV(int i) const {return m_N_g_1GeV[i];}
    /**m_N_g_2GeV leaf*/
    int get_N_g_2GeV(int i) const {return m_N_g_2GeV[i];}
    /**m_CDCTOP_jitter leaf*/
    int get_CDCTOP_jitter() const {return m_CDCTOP_jitter;}
    /**m_c_b2b_1to9 leaf*/
    int get_c_b2b_1to9(int i) const {return m_c_b2b_1to9[i];}
    /**m_c_b2b_1to7 leaf*/
    int get_c_b2b_1to7(int i) const {return m_c_b2b_1to7[i];}
    /**m_c_b2b_1to5 leaf*/
    int get_c_b2b_1to5(int i) const {return  m_c_b2b_1to5[i];}
    /**m_c_b2b_1to3 leaf*/
    int get_c_b2b_1to3(int i) const {return m_c_b2b_1to3[i];}
    /**m_tc_b2b_1to9 leaf*/
    int get_tc_b2b_1to9(int i) const {return m_tc_b2b_1to9[i];}
    /**m_tc_b2b_1to7 leaf*/
    int get_tc_b2b_1to7(int i) const {return m_tc_b2b_1to7[i];}
    /**m_tc_b2b_1to5 leaf*/
    int get_tc_b2b_1to5(int i) const {return  m_tc_b2b_1to5[i];}
    /**m_tc_b2b_1to3 leaf*/
    int get_tc_b2b_1to3(int i) const {return  m_tc_b2b_1to3[i];}
    /**m_CDC3D_jitter leaf*/
    int get_CDC3D_jitter() const {return  m_CDC3D_jitter;}
    /**m_CDCNN_jitter leaf*/
    int get_CDCNN_jitter() const {return  m_CDCNN_jitter;}

    /**m_slot_CDC integer leaf*/
    int get_slot_CDC_int() const {return  m_slot_CDC_int;}
    /**m_slot_CDC map leaf*/
    bool get_slot_CDC(int i) const {return m_slot_CDC[i];}
    /**m_slot_TOP integer leaf*/
    int get_slot_TOP_int() const {return m_slot_TOP_int;}
    /**m_slot_TOP map leaf*/
    bool get_slot_TOP(int i) const {return m_slot_TOP[i];}
    /**m_N_TOPmatch leaf*/
    int get_N_TOPmatch() const {return  m_N_TOPmatch;}

    /**m_unamed1 leaf*/
    int get_unamed1() const {return m_unamed1;}
    /**m_unamed2 leaf*/
    int get_unamed2() const {return m_unamed2;}
    /**m_coincidence_2DECL leaf*/
    int get_concidence_2DECL() const {return m_coincidence_2DECL;}
    /**m_matched_2DECL leaf*/
    int get_matched_2DECL() const {return m_matched_2DECL;}

    /**m_theta_ECL leaf*/
    int get_theta_ECL(int i) const {return m_theta_ECL[i];}
    /**m_phi_ECL leaf*/
    int get_phi_ECL(int i) const {return m_phi_ECL[i];}
    /**m_t_ECL leaf*/
    int get_t_ECL(int i) const {return m_t_ECL[i];}
    /**m_E_ECL leaf*/
    int get_E_ECL(int i) const {return m_E_ECL[i];}
    /**m_clk_ECL leaf*/
    int get_clk_ECL(int i) const {return m_clk_ECL[i];}
    /**m_1GeV_flag_ECL leaf*/
    int get_1GeV_ECL(int i) const {return m_1GeV_ECL[i];}
    /**m_2GeV_flag_ECL leaf*/
    int get_2GeV_ECL(int i) const {return m_2GeV_ECL[i];}

    /**#wire hit in SL0*/
    int get_wire_tsf0() const {return m_wire_tsf0;}
    /**#wire hit in SL1*/
    int get_wire_tsf1() const {return m_wire_tsf1;}
    /**#wire hit in SL2*/
    int get_wire_tsf2() const {return m_wire_tsf2;}
    /**#wire hit in SL3*/
    int get_wire_tsf3() const {return m_wire_tsf3;}
    /**#wire hit in SL4*/
    int get_wire_tsf4() const {return m_wire_tsf4;}

    /**m_map_TSF0 integer leaf*/
    int get_map_TSF0_int(int i) const {return m_map_TSF0_int[i];}
    /**m_map_TSF2 integer leaf*/
    int get_map_TSF2_int(int i) const {return m_map_TSF2_int[i];}
    /**m_map_TSF4 integer leaf*/
    int get_map_TSF4_int(int i) const {return m_map_TSF4_int[i];}
    /**m_map_TSF1 integer leaf*/
    int get_map_TSF1_int(int i) const {return m_map_TSF1_int[i];}
    /**m_map_TSF3 integer leaf*/
    int get_map_TSF3_int(int i) const {return m_map_TSF3_int[i];}
    /**m_map_veto integer leaf*/
    int get_map_veto_int(int i) const {return m_map_veto_int[i];}
    /**m_map_ST integer leaf*/
    int get_map_ST_int(int i) const {return m_map_ST_int[i];}
    /**m_map_ST2 integer leaf*/
    int get_map_ST2_int(int i) const {return m_map_ST2_int[i];}
    /**m_map_TSF0 map leaf*/
    bool get_map_TSF0(int i) const {return m_map_TSF0[i];}
    /**m_map_TSF2 map leaf*/
    bool get_map_TSF2(int i) const {return m_map_TSF2[i];}
    /**m_map_TSF4 map leaf*/
    bool get_map_TSF4(int i) const {return m_map_TSF4[i];}
    /**m_map_TSF1 map leaf*/
    bool get_map_TSF1(int i) const {return m_map_TSF1[i];}
    /**m_map_TSF3 map leaf*/
    bool get_map_TSF3(int i) const {return m_map_TSF3[i];}
    /**m_map_veto map leaf*/
    bool get_map_veto(int i) const {return m_map_veto[i];}
    /**m_map_ST map leaf*/
    bool get_map_ST(int i) const {return m_map_ST[i];}
    /**m_map_ST2 map leaf*/
    bool get_map_ST2(int i) const {return m_map_ST2[i];}

    /**m_X leaf*/
    int get_X() const {return  m_X;}
    /**m_X_0 leaf*/
    int get_X_0() const {return m_X_0;}
    /**m_X_1 leaf*/
    int get_X_1() const {return m_X_1;}
    /**m_X_2 leaf*/
    int get_X_2() const {return m_X_2;}
    /**m_N_cluster leaf*/
    int get_N_cluster() const {return m_N_cluster;}
    /**m_N_cluster_0 leaf*/
    int get_N_cluster_0() const {return m_N_cluster_0;}
    /**m_N_cluster_1 leaf*/
    int get_N_cluster_1() const {return m_N_cluster_1;}
    /**m_N_cluster_2 leaf*/
    int get_N_cluster_2() const {return m_N_cluster_2;}
    /**m_revo_fam leaf*/
    int get_revo_fam() const {return m_revo_fam;}
    /**m_ECL_timing leaf*/
    int get_ECL_timing() const {return m_ECL_timing;}
    /**m_ECL_trg_0 leaf*/
    int get_ECL_trg_0() const {return m_ECL_trg_0;}
    /**m_ECL_trg_1 leaf*/
    int get_ECL_trg_1() const {return m_ECL_trg_1;}
    /**m_ECL_trg_2 leaf*/
    int get_ECL_trg_2() const {return m_ECL_trg_2;}
    /**m_TOP_trg leaf*/
    int get_TOP_trg() const {return m_TOP_trg;}
    /**m_TSFL1_count leaf*/
    int get_TSFL1_count() const {return m_TSFL1_count;}
    /**m_slot64b_TOP_int_1 leaf*/
    int get_slot64b_TOP_int_1() const {return m_slot64b_TOP_int_1;}
    /**m_slot64b_TOP_int_0 leaf*/
    int get_slot64b_TOP_int_0() const {return m_slot64b_TOP_int_0;}
    /**m_ECLL1_count leaf*/
    int get_ECLL1_count() const {return m_ECLL1_count;}
    /**m_CDCL1_count leaf*/
    int get_CDCL1_count() const {return m_CDCL1_count;}
    /**m_TOPL1_count leaf*/
    int get_TOPL1_count() const {return m_TOPL1_count;}
    /**m_KLML1_count leaf*/
    int get_KLML1_count() const {return m_KLML1_count;}
    /**m_CDC3DL1_count leaf*/
    int get_CDC3DL1_count() const {return m_CDC3DL1_count;}
    /**m_CDCNNL1_count leaf*/
    int get_CDCNNL1_count() const {return m_CDCNNL1_count;}
    /**m_unamed3 leaf*/
    int get_unamed3() const {return m_unamed3;}
    /**m_revoclk1 leaf*/
    int get_revoclk1() const {return m_revoclk1;}
    /**m_aaaa leaf*/
    int get_aaaa() const {return m_aaaa;}
    /**m_charge_flag leaf*/
    int get_charge_flag() const {return m_charge_flag;}
    /**m_charge_sum leaf*/
    int get_charge_sum() const {return m_charge_sum;}
    /**m_firmid leafs*/
    int get_firmid() const {return m_firmid;}
    /**m_firmver leafs*/
    int get_firmver() const {return m_firmver;}
    /**m_evt leafs*/
    int get_evt() const {return m_evt;}
    /**m_clk leafs*/
    int get_clk() const {return m_clk;}
    /**m_coml1 leafs*/
    int get_coml1() const {return m_coml1;}
    /**m_b2ldly leafs*/
    int get_b2ldly() const {return m_b2ldly;}
    /**m_maxrvc leafs*/
    int get_maxrvc() const {return m_maxrvc;}
    //end get

    //set
    /**m_dddd leaf*/
    void set_dddd(int dddd) {m_dddd = dddd;}
    /**m_unamed leaf*/
    void set_unamed(int unamed) {m_unamed = unamed;}
    /**m_revoclk leaf*/
    void set_revoclk(int revoclk) {m_revoclk = revoclk;}
    /**m_ECL_trg leaf*/
    void set_ECL_trg(int ECLtrg) {m_ECL_trg = ECLtrg; }
    /**m_ECL_trg_delay leaf*/
    void set_ECL_trg_delay(int ECL_trg_delay) {m_ECL_trg_delay = ECL_trg_delay; }
    /**m_CDCECL_jitter leaf*/
    void set_CDCECL_jitter(int CDCECL_jitter)  {m_CDCECL_jitter = CDCECL_jitter;}
    /**m_phi_CDC integer leaf*/
    void set_phi_CDC_int(int i, int phi_CDC_int)  {m_phi_CDC_int[i] = phi_CDC_int;}
    /**m_phi_CDC map leaf*/
    void set_phi_CDC(int i, bool phi_CDC) {m_phi_CDC[i] = phi_CDC;}
    /**m_phi_i integer leaf*/
    void set_phi_i_int(int i, int phi_i_int) { m_phi_i_int[i] = phi_i_int; }
    /**m_phi_i map leaf*/
    void set_phi_i(int i, bool phi_i) { m_phi_i[i] = phi_i; }
    /**m_N_track leaf*/
    void set_N_track(int N_track) { m_N_track = N_track;}
    /**m_N_track_cut leaf*/
    void set_N_track_cut(int N_track_cut) { m_N_track_cut = N_track_cut;}
    /**m_N_track_3D leaf*/
    void set_N_track_3D(int N_track_3D)  { m_N_track_3D = N_track_3D; }
    /**m_N_track_NN leaf*/
    void set_N_track_NN(int N_track_NN)  { m_N_track_NN = N_track_NN ;}
    /**m_N_ST leaf*/
    void set_N_ST(int N_ST) { m_N_ST = N_ST; }
    /**m_b2b_1to3 leaf*/
    void set_b2b_1to3(int b2b_1to3) { m_b2b_1to3 = b2b_1to3; }
    /**m_b2b_1to5 leaf*/
    void set_b2b_1to5(int b2b_1to5) { m_b2b_1to5 = b2b_1to5; }
    /**m_b2b_1to7 leaf*/
    void set_b2b_1to7(int b2b_1to7)  { m_b2b_1to7 = b2b_1to7; }
    /**m_b2b_1to9 leaf*/
    void set_b2b_1to9(int b2b_1to9)  { m_b2b_1to9 = b2b_1to9; }
    /**m_opening leaf*/
    void set_opening(int opening) { m_opening = opening; }
    /**m_cc leaf*/
    void set_cc(int cc) {m_cc = cc;}
    /**m_unamed0 leaf*/
    void set_unamed0(int unamed0)  { m_unamed0 = unamed0; }
    /**m_KLM_trg leaf*/
    void set_KLM_trg(int KLM_trg) { m_KLM_trg = KLM_trg; }
    /**m_matched_2DKLM leaf*/
    void set_matched_2DKLM(int matched_2DKLM) { m_matched_2DKLM = matched_2DKLM; }
    /**m_CDCKLM_jitter leaf*/
    void set_CDCKLM_jitter(int CDCKLM_jitter) {m_CDCKLM_jitter = CDCKLM_jitter;}
    /**m_KLMCDC_jitter leaf*/
    void set_KLMCDC_jitter(int KLMCDC_jitter) { m_KLMCDC_jitter = KLMCDC_jitter;}
    /**m_sector_KLM integer leaf*/
    void set_sector_KLM_int(int sector_KLM_int)  { m_sector_KLM_int = sector_KLM_int;}
    /**m_sector_KLM map leaf*/
    void set_sector_KLM(int i, bool sector_KLM)  { m_sector_KLM[i] = sector_KLM;}
    /**m_sector_CDC integer leaf*/
    void set_sector_CDC_int(int sector_CDC_int)  { m_sector_CDC_int = sector_CDC_int;}
    /**m_sector_CDC map */
    void set_sector_CDC(int i, bool sector_CDC) { m_sector_CDC[i] = sector_CDC ;}
    /**m_N_KLMmatch leaf*/
    void set_N_KLMmatch(int N_KLMmatch) { m_N_KLMmatch = N_KLMmatch; }
    /**m_samehem leaf*/
    void set_samehem(int i, int samehem)  {  m_samehem[i] = samehem; }
    /**m_opphem leaf*/
    void set_opphem(int i, int opphem)  { m_opphem[i] = opphem; }
    /**m_N_match_2GeV leaf*/
    void set_N_match_2GeV(int i, int N_match_2GeV)  { m_N_match_2GeV[i] = N_match_2GeV; }
    /**m_N_match leaf*/
    void set_N_match(int i, int N_match) { m_N_match[i] = N_match; }
    /**m_N_g_1GeV leaf*/
    void set_N_g_1GeV(int i, int N_g_1GeV) { m_N_g_1GeV[i] = N_g_1GeV; }
    /**m_N_g_2GeV leaf*/
    void set_N_g_2GeV(int i, int N_g_2GeV) { m_N_g_2GeV[i] = N_g_2GeV; }
    /**m_CDCTOP_jitter leaf*/
    void set_CDCTOP_jitter(int CDCTOP_jitter)  { m_CDCTOP_jitter = CDCTOP_jitter; }
    /**m_c_b2b_1to9 leaf*/
    void set_c_b2b_1to9(int i, int c_b2b_1to9)  { m_c_b2b_1to9[i] = c_b2b_1to9; }
    /**m_c_b2b_1to7 leaf*/
    void set_c_b2b_1to7(int i, int c_b2b_1to7)  { m_c_b2b_1to7[i] = c_b2b_1to7; }
    /**m_c_b2b_1to5 leaf*/
    void set_c_b2b_1to5(int i, int c_b2b_1to5)  { m_c_b2b_1to5[i] = c_b2b_1to5; }
    /**m_c_b2b_1to3 leaf*/
    void set_c_b2b_1to3(int i, int c_b2b_1to3)  { m_c_b2b_1to3[i] = c_b2b_1to3; }
    /**m_tc_b2b_1to9 leaf*/
    void set_tc_b2b_1to9(int i, int tc_b2b_1to9)  { m_tc_b2b_1to9[i] = tc_b2b_1to9; }
    /**m_tc_b2b_1to7 leaf*/
    void set_tc_b2b_1to7(int i, int tc_b2b_1to7) {m_tc_b2b_1to7[i] = tc_b2b_1to7; }
    /**m_tc_b2b_1to5 leaf*/
    void set_tc_b2b_1to5(int i, int tc_b2b_1to5)  { m_tc_b2b_1to5[i] = tc_b2b_1to5; }
    /**m_tc_b2b_1to3 leaf*/
    void set_tc_b2b_1to3(int i, int tc_b2b_1to3)  { m_tc_b2b_1to3[i] = tc_b2b_1to3; }
    /**m_CDC3D_jitter leaf*/
    void set_CDC3D_jitter(int CDC3D_jitter)  { m_CDC3D_jitter = CDC3D_jitter; }
    /**m_CDCNN_jitter leaf*/
    void set_CDCNN_jitter(int CDCNN_jitter) { m_CDCNN_jitter = CDCNN_jitter; }

    /**m_slot_CDC integer leaf*/
    void set_slot_CDC_int(int slot_CDC_int) { m_slot_CDC_int = slot_CDC_int; }
    /**m_slot_CDC map leaf*/
    void set_slot_CDC(int i, bool slot_CDC) { m_slot_CDC[i] = slot_CDC; }
    /**m_slot_TOP integer leaf*/
    void set_slot_TOP_int(int slot_TOP_int) { m_slot_TOP_int = slot_TOP_int; }
    /**m_slot_TOP map leaf*/
    void set_slot_TOP(int i, bool slot_TOP)  { m_slot_TOP[i] = slot_TOP; }
    /**m_N_TOPmatch leaf*/
    void set_N_TOPmatch(int N_TOPmatch)  { m_N_TOPmatch = N_TOPmatch; }

    /**m_unamed1 leaf*/
    void set_unamed1(int unamed1) { m_unamed1 = unamed1; }
    /**m_unamed2 leaf*/
    void set_unamed2(int unamed2) { m_unamed2 = unamed2; }
    /**m_coincidence_2DECL leaf*/
    void set_concidence_2DECL(int coincidence_2DECL) { m_coincidence_2DECL = coincidence_2DECL; }
    /**m_matched_2DECL leaf*/
    void set_matched_2DECL(int matched_2DECL) { m_matched_2DECL = matched_2DECL; }

    /**m_theta_ECL leaf*/
    void set_theta_ECL(int i, int theta_ECL)  { m_theta_ECL[i] = theta_ECL;}
    void Addto_theta_ECL(int theta_ECL)  { m_theta_ECL.push_back(theta_ECL);}
    /**m_phi_ECL leaf*/
    void set_phi_ECL(int i, int phi_ECL) { m_phi_ECL[i] = phi_ECL;}
    void Addto_phi_ECL(int phi_ECL) { m_phi_ECL.push_back(phi_ECL) ;}
    /**m_t_ECL leaf*/
    void set_t_ECL(int i, int t_ECL)  { m_t_ECL[i] = t_ECL;}
    void Addto_t_ECL(int t_ECL)  { m_t_ECL.push_back(t_ECL) ;}
    /**m_E_ECL leaf*/
    void set_E_ECL(int i, int E_ECL)  { m_E_ECL[i] = E_ECL;}
    void Addto_E_ECL(int E_ECL)  { m_E_ECL.push_back(E_ECL) ;}
    /**m_clk_ECL leaf*/
    void set_clk_ECL(int i, int clk_ECL) { m_clk_ECL[i] = clk_ECL; }
    void Addto_clk_ECL(int clk_ECL) { m_clk_ECL.push_back(clk_ECL); }
    /**m_1GeV_flag_ECL leaf*/
    void set_1GeV_ECL(int i, int c1GeV_ECL)  { m_1GeV_ECL[i] = c1GeV_ECL; }
    void Addto_1GeV_ECL(int c1GeV_ECL)  { m_1GeV_ECL.push_back(c1GeV_ECL); }
    /**m_2GeV_flag_ECL leaf*/
    void set_2GeV_ECL(int i, int c2GeV_ECL)  { m_2GeV_ECL[i] = c2GeV_ECL; }
    void Addto_2GeV_ECL(int c2GeV_ECL)  { m_2GeV_ECL.push_back(c2GeV_ECL) ; }

    /**#wire hit in SL0*/
    void set_wire_tsf0(int wire_tsf0) { m_wire_tsf0 = wire_tsf0; }
    /**#wire hit in SL1*/
    void set_wire_tsf1(int wire_tsf1) { m_wire_tsf1 = wire_tsf1; }
    /**#wire hit in SL2*/
    void set_wire_tsf2(int wire_tsf2) { m_wire_tsf2 = wire_tsf2; }
    /**#wire hit in SL3*/
    void set_wire_tsf3(int wire_tsf3) { m_wire_tsf3 = wire_tsf3; }
    /**#wire hit in SL4*/
    void set_wire_tsf4(int wire_tsf4) { m_wire_tsf4 = wire_tsf4; }

    /**m_map_TSF0 integer leaf*/
    void set_map_TSF0_int(int i, int map_TSF0_int)  { m_map_TSF0_int[i] = map_TSF0_int; }
    /**m_map_TSF2 integer leaf*/
    void set_map_TSF2_int(int i, int map_TSF2_int)  { m_map_TSF2_int[i] = map_TSF2_int; }
    /**m_map_TSF4 integer leaf*/
    void set_map_TSF4_int(int i, int map_TSF4_int)  { m_map_TSF4_int[i] = map_TSF4_int; }
    /**m_map_TSF1 integer leaf*/
    void set_map_TSF1_int(int i, int map_TSF1_int)  { m_map_TSF1_int[i] = map_TSF1_int; }
    /**m_map_TSF3 integer leaf*/
    void set_map_TSF3_int(int i, int map_TSF3_int)  { m_map_TSF3_int[i] = map_TSF3_int; }
    /**m_map_veto integer leaf*/
    void set_map_veto_int(int i, int map_veto_int)  { m_map_veto_int[i] = map_veto_int; }
    /**m_map_ST integer leaf*/
    void set_map_ST_int(int i, int map_ST_int)   { m_map_ST_int[i] = map_ST_int; }
    /**m_map_ST2 integer leaf*/
    void set_map_ST2_int(int i, int map_ST2_int)  { m_map_ST2_int[i] = map_ST2_int; }
    /**m_map_TSF0 map leaf*/
    void set_map_TSF0(int i, bool map_TSF0)  { m_map_TSF0[i] = map_TSF0; }
    /**m_map_TSF2 map leaf*/
    void set_map_TSF2(int i, bool map_TSF2)  { m_map_TSF2[i] = map_TSF2; }
    /**m_map_TSF4 map leaf*/
    void set_map_TSF4(int i, bool map_TSF4)  { m_map_TSF4[i] = map_TSF4; }
    /**m_map_TSF1 map leaf*/
    void set_map_TSF1(int i, bool map_TSF1)  { m_map_TSF1[i] = map_TSF1; }
    /**m_map_TSF3 map leaf*/
    void set_map_TSF3(int i, bool map_TSF3)  { m_map_TSF3[i] = map_TSF3; }
    /**m_map_veto map leaf*/
    void set_map_veto(int i, bool map_veto)  { m_map_veto[i] = map_veto; }
    /**m_map_ST map leaf*/
    void set_map_ST(int i, bool map_ST)  { m_map_ST[i] = map_ST; }
    /**m_map_ST2 map leaf*/
    void set_map_ST2(int i, bool map_ST2)  { m_map_ST2[i] = map_ST2; }

    /**m_X leaf*/
    void set_X(int X)  { m_X = X; }
    /**m_X_0 leaf*/
    void set_X_0(int X_0)  { m_X_0 = X_0; }
    /**m_X_1 leaf*/
    void set_X_1(int X_1)  { m_X_1 = X_1; }
    /**m_X_2 leaf*/
    void set_X_2(int X_2)  { m_X_2 = X_2; }
    /**m_N_cluster leaf*/
    void set_N_cluster(int N_cluster)  { m_N_cluster = N_cluster; }
    /**m_N_cluster_0 leaf*/
    void set_N_cluster_0(int N_cluster_0)  { m_N_cluster_0 = N_cluster_0; }
    /**m_N_cluster_1 leaf*/
    void set_N_cluster_1(int N_cluster_1)  { m_N_cluster_1 = N_cluster_1; }
    /**m_N_cluster_2 leaf*/
    void set_N_cluster_2(int N_cluster_2)  { m_N_cluster_2 = N_cluster_2; }
    /**m_revo_fam leaf*/
    void set_revo_fam(int revo_fam)  { m_revo_fam = revo_fam; }
    /**m_ECL_timing leaf*/
    void set_ECL_timing(int ECL_timing)  { m_ECL_timing = ECL_timing; }
    /**m_ECL_trg_0 leaf*/
    void set_ECL_trg_0(int ECL_trg_0)  { m_ECL_trg_0 = ECL_trg_0; }
    /**m_ECL_trg_1 leaf*/
    void set_ECL_trg_1(int ECL_trg_1)  { m_ECL_trg_1 = ECL_trg_1; }
    /**m_ECL_trg_2 leaf*/
    void set_ECL_trg_2(int ECL_trg_2)  { m_ECL_trg_2 = ECL_trg_2;}
    /**m_TOP_trg leaf*/
    void set_TOP_trg(int TOP_trg)  { m_TOP_trg = TOP_trg;}
    /**m_TSFL1_count leaf*/
    void set_TSFL1_count(int TSFL1_count)  { m_TSFL1_count = TSFL1_count;}
    /**m_slot64b_TOP_int_1 leaf*/
    void set_slot64b_TOP_int_1(int slot64b_TOP_int_1)  { m_slot64b_TOP_int_1 = slot64b_TOP_int_1;}
    /**m_slot64b_TOP_int_0 leaf*/
    void set_slot64b_TOP_int_0(int slot64b_TOP_int_0)  { m_slot64b_TOP_int_0 = slot64b_TOP_int_0;}
    /**m_ECLL1_count leaf*/
    void set_ECLL1_count(int ECLL1_count)  { m_ECLL1_count = ECLL1_count;}
    /**m_CDCL1_count leaf*/
    void set_CDCL1_count(int CDCL1_count)  { m_CDCL1_count = CDCL1_count ;}
    /**m_TOPL1_count leaf*/
    void set_TOPL1_count(int TOPL1_count)  { m_TOPL1_count = TOPL1_count;}
    /**m_KLML1_count leaf*/
    void set_KLML1_count(int KLML1_count)  { m_KLML1_count = KLML1_count;}
    /**m_CDC3DL1_count leaf*/
    void set_CDC3DL1_count(int CDC3DL1_count)  { m_CDC3DL1_count = CDC3DL1_count;}
    /**m_CDCNNL1_count leaf*/
    void set_CDCNNL1_count(int CDCNNL1_count)  { m_CDCNNL1_count = CDCNNL1_count;}
    /**m_unamed3 leaf*/
    void set_unamed3(int unamed3)  { m_unamed3 = unamed3;}
    /**m_revoclk1 leaf*/
    void set_revoclk1(int revoclk1)  { m_revoclk1 = revoclk1;}
    /**m_aaaa leaf*/
    void set_aaaa(int aaaa)  { m_aaaa = aaaa;}
    /**m_charge_flag leaf*/
    void set_charge_flag(int charge_flag)  { m_charge_flag = charge_flag;}
    /**m_charge_sum leaf*/
    void set_charge_sum(int charge_sum) { m_charge_sum = charge_sum;}
    /**m_firmid leafs*/
    void set_firmid(int firmid)  { m_firmid = firmid;}
    /**m_firmver leafs*/
    void set_firmver(int firmver)  { m_firmver = firmver;}
    /**m_evt leafs*/
    void set_evt(int evt)  { m_evt = evt;}
    /**m_clk leafs*/
    void set_clk(int clk)  { m_clk = clk;}
    /**m_coml1 leafs*/
    void set_coml1(int coml1)  { m_coml1 = coml1;}
    /**m_b2ldly leafs*/
    void set_b2ldly(int b2ldly)  { m_b2ldly = b2ldly;}
    /**m_maxrvc leafs*/
    void set_maxrvc(int maxrvc)  { m_maxrvc = maxrvc;}
    /**clear vectors*/
    void ClearVectors()
    {
      m_E_ECL.clear();
      m_t_ECL.clear();
      m_clk_ECL.clear();
      m_theta_ECL.clear();
      m_phi_ECL.clear();
      m_1GeV_ECL.clear();
      m_2GeV_ECL.clear();
    }


  private:
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

    /**#wire hit in SL0*/
    int m_wire_tsf0;
    /**#wire hit in SL2*/
    int m_wire_tsf2;
    /**#wire hit in SL4*/
    int m_wire_tsf4;
    /**#wire hit in SL1*/
    int m_wire_tsf1;
    /**#wire hit in SL3*/
    int m_wire_tsf3;

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
    ClassDef(TRGGRLUnpackerStore, 2);

  };

} // end namespace Belle2

#endif
