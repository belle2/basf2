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
// Filename : TRGGRLInfo.h
// Section  : TRG GRL
// Owner    : Ke LI, Junhao Yin, Chunhua Li
// Email    : like@ihep.ac.cn, yinjh@ihep.ac.cn,
//            chunhua.li@unimelb.edu.au
//-----------------------------------------------------------
// Description : A class to save TRG GRL information of event.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#pragma once
#include <TObject.h>

namespace Belle2 {

  /// a class to store TRGGRL TSIM info
  class TRGGRLInfo : public TObject {
  public:

    /// Constructor
    TRGGRLInfo():
      m_n_2dfinder_track(-1),
      m_n_2dfitter_track(-1),
      m_n_3dfitter_track(-1),
      m_n_3dfitter_track_Z10(-1),
      m_n_3dfitter_track_Z25(-1),
      m_n_3dfitter_track_Z35(-1),
      m_n_NN_track(-1),
      m_n_NN_track_Z20(-1),
      m_n_NN_track_Z40(-1),
      m_n_NN_track_STT(-1),
      m_n_2dmatch_track(-1),
      m_n_3dmatch_track(-1),
      m_n_cluster(-1),
      m_n_neutral_cluster(-1),
      m_n_high_300_cluster(-1),
      m_n_high_1000_cluster(-1),
      m_n_high_1000_cluster415(-1),
      m_n_high_1000_cluster2316(-1),
      m_n_high_1000_cluster117(-1),
      m_n_high_2000_cluster(-1),
      m_n_high_2000_cluster414(-1),
      m_n_high_2000_cluster231516(-1),
      m_n_high_2000_cluster117(-1),
      m_n_high_2000_endcap_cluster(-1),
      m_nbbc(-1),
      m_nbbtc(-1),
      m_n_klm_track(-1),
      m_n_klm_hit(-1),
      m_bhabhaveto(-1),
      m_eclbhabhaveto(-1),
      m_n_PhiPairHigh(-1),
      m_n_PhiPairLow(-1),
      m_n_3DPair(-1),
      m_n_SameHem1Trk(-1),
      m_n_OppHem1Trk(-1),
      m_Trk_b2b_1to3(-1),
      m_Trk_b2b_1to5(-1),
      m_Trk_b2b_1to7(-1),
      m_Trk_b2b_1to9(-1),
      m_Trk_open90(-1),
      m_Trk_open30(-1),
      m_cluster_b2b_1to3(-1),
      m_cluster_b2b_1to5(-1),
      m_cluster_b2b_1to7(-1),
      m_cluster_b2b_1to9(-1),
      m_Trkcluster_b2b_1to3(-1),
      m_Trkcluster_b2b_1to5(-1),
      m_Trkcluster_b2b_1to7(-1),
      m_Trkcluster_b2b_1to9(-1),
      m_eed(-1),
      m_fed(-1),
      m_fp(-1),
      m_eeb(-1),
      m_fep(-1),
      m_n_st(-1),
      m_n_st_fwd(-1),
      m_n_st_bwd(-1),
      m_s2s3(-1),
      m_s2s5(-1),
      m_s2so(-1),
      m_s2s30(-1),
      m_s2f3(-1),
      m_s2f5(-1),
      m_s2fo(-1),
      m_s2f30(-1),
      m_bwdsb(-1),
      m_bwdnb(-1),
      m_fwdsb(-1),
      m_fwdnb(-1),
      m_brlfb(-1),
      m_brlnb(-1),
      m_n_secl(-1),
      m_n_secl_fwd(-1),
      m_n_secl_bwd(-1),
      m_n_sklm(-1),
      m_n_sklm_fwd(-1),
      m_n_sklm_bwd(-1),
      m_n_it(-1),
      m_i2fo(-1),
      m_i2io(-1),
      m_n_iecl(-1),
      m_n_iklm(-1),
      m_n_eecleklm(-1)
    {}
    ~TRGGRLInfo() {}


    /**get the number of 2D tracks*/
    int getN2Dfindertrk() const  {return m_n_2dfinder_track;}
    /**get the number of 2D fitter tracks*/
    int getN2Dfittertrk() const  {return m_n_2dfitter_track;}
    /**get the number of 3D charged tracks*/
    int getN3Dfittertrk() const  {return m_n_3dfitter_track;}
    /**get the number of 3D charged tracks*/
    int getN3DfittertrkZ10() const  {return m_n_3dfitter_track_Z10;}
    /**get the number of 3D charged tracks*/
    int getN3DfittertrkZ25() const  {return m_n_3dfitter_track_Z25;}
    /**get the number of 3D charged tracks*/
    int getN3DfittertrkZ35() const  {return m_n_3dfitter_track_Z35;}
    /**get the number of NN charged tracks*/
    int getNNNtrk() const  {return m_n_NN_track;}
    /**get the number of NN charged tracks Z0<20cm*/
    int getNNNtrkZ20() const  {return m_n_NN_track_Z20;}
    /**get the number of NN charged tracks Z0<40cm*/
    int getNNNtrkZ40() const  {return m_n_NN_track_Z40;}
    /**get the number of NN STT Z0<15cm, P>0.7GeV*/
    int getNNNtrkSTT() const  {return m_n_NN_track_STT;}
    /**get the number of 2D matched tracks*/
    int getN2Dmatchtrk() const {return m_n_2dmatch_track;}
    /**get the number of 3D matched tracks*/
    int getN3Dmatchtrk() const {return m_n_3dmatch_track;}
    /**get the number of 3D NN tracks*/
    //int getNNNmatchtrk() const {return m_n_NNmatch_track;}
    /**get the number of eclcluster*/
    int getNcluster() const {return m_n_cluster;}
    /**get the number of eclclusters >300MeV*/
    int getNhig300cluster() const {return m_n_high_300_cluster;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh1GeVcluster() const {return m_n_high_1000_cluster;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh1GeVcluster415() const {return m_n_high_1000_cluster415;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh1GeVcluster2316() const {return m_n_high_1000_cluster2316;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh1GeVcluster117() const {return m_n_high_1000_cluster117;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh2GeVcluster() const {return m_n_high_2000_cluster;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh2GeVcluster414() const {return m_n_high_2000_cluster414;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh2GeVcluster231516() const {return m_n_high_2000_cluster231516;}
    /**get the number of eclcluster >1GeV, exclude TCID 1,2,17*/
    int getNhigh2GeVcluster117() const {return m_n_high_2000_cluster117;}
    /**get the number of eclclusters without associated tracks*/
    int getNneucluster() const {return m_n_neutral_cluster;}
    /**get the number of klm tracks*/
    int getNklmtrk() const {return m_n_klm_track;}
    /**get the number of klm hits*/
    int getNklmhit() const {return m_n_klm_hit;}
    /**get two track bhabha logic*/
    int getBhabhaVeto() const {return m_bhabhaveto;}
    /**get eclbhabha*/
    int geteclBhabhaVeto() const {return m_eclbhabhaveto;}
    /**get PhiPairHigh*/
    int getPhiPairHigh() const {return m_n_PhiPairHigh;}
    /**get PhiPairLow*/
    int getPhiPairLow() const {return m_n_PhiPairLow;}
    /**3D pair*/
    int get3DPair() const {return m_n_3DPair;}
    /**NSameHem1Trk*/
    int getNSameHem1Trk() const {return m_n_SameHem1Trk;}
    /**NOppHem1Trk*/
    int getNOppHem1Trk() const {return m_n_OppHem1Trk;}
    /**get Trk_b2b_1to3*/
    int getTrk_b2b_1to3() const {return m_Trk_b2b_1to3;}
    /**get Trk_b2b_1to5*/
    int getTrk_b2b_1to5() const {return m_Trk_b2b_1to5;}
    /**get Trk_b2b_1to7*/
    int getTrk_b2b_1to7() const {return m_Trk_b2b_1to7;}
    /**get Trk_b2b_1to9*/
    int getTrk_b2b_1to9() const {return m_Trk_b2b_1to9;}
    /**get Trk_open90*/
    int getTrk_open90() const {return m_Trk_open90;}
    /**get Trk_open30*/
    int getTrk_open30() const {return m_Trk_open30;}
    /**get cluster_b2b_1to3*/
    int getcluster_b2b_1to3() const {return m_cluster_b2b_1to3;}
    /**get cluster_b2b_1to5*/
    int getcluster_b2b_1to5() const {return m_cluster_b2b_1to5;}
    /**get cluster_b2b_1to7*/
    int getcluster_b2b_1to7() const {return m_cluster_b2b_1to7;}
    /**get cluster_b2b_1to9*/
    int getcluster_b2b_1to9() const {return m_cluster_b2b_1to9;}
    /**get Trkcluster_b2b_1to3*/
    int getTrkcluster_b2b_1to3() const {return m_Trkcluster_b2b_1to3;}
    /**get Trkcluster_b2b_1to5*/
    int getTrkcluster_b2b_1to5() const {return m_Trkcluster_b2b_1to5;}
    /**get Trkcluster_b2b_1to7*/
    int getTrkcluster_b2b_1to7() const {return m_Trkcluster_b2b_1to7;}
    /**get Trkcluster_b2b_1to9*/
    int getTrkcluster_b2b_1to9() const {return m_Trkcluster_b2b_1to9;}
    /**get eed: two matched & cluster b2b */
    int geteed() const {return m_eed;}
    /**get fed: one track & one matched & cluster b2b */
    int getfed() const {return m_fed;}
    /**get fp: one track & track-cluster b2b */
    int getfp() const {return m_fp;}
    /**get eeb: two matched & track b2b */
    int geteeb() const {return m_eeb;}
    /**get fep: one track & one matched & track-cluster b2b */
    int getfep() const {return m_fep;}
    /**get the number of short tracks*/
    int getNshorttrk() const  {return m_n_st;}
    /**get the number of fwd short tracks*/
    int getNshorttrk_fwd() const  {return m_n_st_fwd;}
    /**get the number of bwd short tracks*/
    int getNshorttrk_bwd() const  {return m_n_st_bwd;}
    /**get s2s3: short-to-short b2b, 10 to 30 degress*/
    int gets2s3() const  {return m_s2s3;}
    /**get s2s5: short-to-short b2b, 10 to 50 degress*/
    int gets2s5() const  {return m_s2s5;}
    /**get s2so: short-to-short opening angle > 90 degrees*/
    int gets2so() const  {return m_s2so;}
    /**get s2s30: short-to-short opening angle > 30 degrees*/
    int gets2s30() const  {return m_s2s30;}
    /**get s2f3: short-to-full b2b, 10 to 30 degress*/
    int gets2f3() const  {return m_s2f3;}
    /**get s2f5: short-to-full b2b, 10 to 50 degress*/
    int gets2f5() const  {return m_s2f5;}
    /**get s2fo: short-to-full opening angle > 90 degrees*/
    int gets2fo() const  {return m_s2fo;}
    /**get s2f30: short-to-full opening angle > 30 degrees*/
    int gets2f30() const  {return m_s2f30;}
    /**get bwdsb: short track matched to bhabha cluster in bwd endcap*/
    int getbwdsb() const  {return m_bwdsb;}
    /**get bwdnb: neutral bhabha cluster in bwd endcap*/
    int getbwdnb() const  {return m_bwdnb;}
    /**get fwdsb: short track matched to bhabha cluster in fwd endcap*/
    int getfwdsb() const  {return m_bwdsb;}
    /**get fwdnb: neutral bhabha cluster in fwd endcap*/
    int getfwdnb() const  {return m_bwdnb;}
    /**get brlfb: full track matched to bhabha cluster in barrel*/
    int getbrlfb() const  {return m_brlfb;}
    /**get brlnb: neutral bhabha cluster in barrel*/
    int getbrlnb() const  {return m_brlnb;}
    /**get the number of ecl matched short tracks*/
    int getNsecl() const  {return m_n_secl;}
    /**get the number of ecl matched fwd short tracks*/
    int getNsecl_fwd() const  {return m_n_secl_fwd;}
    /**get the number of ecl matched bwd short tracks*/
    int getNsecl_bwd() const  {return m_n_secl_bwd;}
    /**get the number of klm matched short tracks*/
    int getNsklm() const  {return m_n_sklm;}
    /**get the number of klm matched fwd short tracks*/
    int getNsklm_fwd() const  {return m_n_sklm_fwd;}
    /**get the number of klm matched bwd short tracks*/
    int getNsklm_bwd() const  {return m_n_sklm_bwd;}
    /**get the number of inner tracks*/
    int getNinnertrk() const  {return m_n_it;}
    /**get i2fo: inner-to-full opening angle > 90 degrees*/
    int geti2fo() const  {return m_i2fo;}
    /**get i2io: inner-to-inner opening angle > 90 degrees*/
    int geti2io() const  {return m_i2io;}
    /**get the number of ecl matched inner tracks*/
    int getNiecl() const  {return m_n_iecl;}
    /**get the number of klm matched inner tracks*/
    int getNiklm() const  {return m_n_iklm;}
    /**get the number of klm matched ecl clusters at the endcap */
    int getNeecleklm() const  {return m_n_eecleklm;}
    /**get GDL input bit */
    bool getInputBits(int i) const {return m_InputBits[i];}

    /**set the number of 2D tracks*/
    void setN2Dfindertrk(int N2Dfindertrk)  {m_n_2dfinder_track = N2Dfindertrk;}
    /**set the number of 2D fitter tracks*/
    void setN2Dfittertrk(int N2Dfittertrk)  {m_n_2dfitter_track = N2Dfittertrk;}
    /**set the number of 3D charged tracks*/
    void setN3Dfittertrk(int N3Dfittertrk)  {m_n_3dfitter_track = N3Dfittertrk;}
    /**set the number of 3D charged tracks*/
    void setN3DfittertrkZ10(int N3DfittertrkZ10)  {m_n_3dfitter_track_Z10 = N3DfittertrkZ10;}
    /**set the number of 3D charged tracks*/
    void setN3DfittertrkZ25(int N3DfittertrkZ25)  {m_n_3dfitter_track_Z25 = N3DfittertrkZ25;}
    /**set the number of 3D charged tracks*/
    void setN3DfittertrkZ35(int N3DfittertrkZ35)  {m_n_3dfitter_track_Z35 = N3DfittertrkZ35;}
    /**set the number of NN charged tracks*/
    void setNNNtrk(int NNNtrk)  {m_n_NN_track = NNNtrk;}
    /**get the number of NN charged tracks Z0<20cm*/
    void setNNNtrkZ20(int NNNtrkZ20)  {m_n_NN_track_Z20 = NNNtrkZ20;}
    /**get the number of NN charged tracks Z0<40cm*/
    void setNNNtrkZ40(int NNNtrkZ40)  {m_n_NN_track_Z40 = NNNtrkZ40;}
    /**get the number of NN charged STT Z0<15sm, P>0.7GeV*/
    void setNNNtrkSTT(int NNNtrkSTT)  {m_n_NN_track_STT = NNNtrkSTT;}
    /**set the number of 2D matched tracks*/
    void setN2Dmatchtrk(int N2Dmatchtrk)  {m_n_2dmatch_track = N2Dmatchtrk;}
    /**set the number of 3D matched tracks*/
    void setN3Dmatchtrk(int N3Dmatchtrk)  {m_n_3dmatch_track = N3Dmatchtrk;}
    /**set the number of clusters*/
    void setNcluster(int Ncluster)  {m_n_cluster = Ncluster;}
    /**set the number of cluster >300 MeV*/
    void setNhigh300cluster(int N300clusterh)  {m_n_high_300_cluster = N300clusterh;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh1GeVcluster(int Nhigh1GeVcluster)  {m_n_high_1000_cluster = Nhigh1GeVcluster;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh1GeVcluster415(int Nhigh1GeVcluster415)  {m_n_high_1000_cluster415 = Nhigh1GeVcluster415;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh1GeVcluster2316(int Nhigh1GeVcluster2316)  {m_n_high_1000_cluster2316 = Nhigh1GeVcluster2316;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh1GeVcluster117(int Nhigh1GeVcluster117)  {m_n_high_1000_cluster117 = Nhigh1GeVcluster117;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh2GeVcluster(int Nhigh2GeVcluster)  {m_n_high_2000_cluster = Nhigh2GeVcluster;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh2GeVcluster414(int Nhigh2GeVcluster414)  {m_n_high_2000_cluster414 = Nhigh2GeVcluster414;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh2GeVcluster231516(int Nhigh2GeVcluster231516)  {m_n_high_2000_cluster231516 = Nhigh2GeVcluster231516;}
    /**set the number of cluster >1 GeV exclude TCID 1,2,17*/
    void setNhigh2GeVcluster117(int Nhigh2GeVcluster117)  {m_n_high_2000_cluster117 = Nhigh2GeVcluster117;}
    /**set the number of cluster w/o associated tracks*/
    void setNneucluster(int Nneucluster)  {m_n_neutral_cluster = Nneucluster;}
    /**set the number of klm tracks*/
    void setNklmtrk(int Nklm_track)  {m_n_klm_track = Nklm_track;}
    /**set the number of klm hits*/
    void setNklmhit(int Nklm_hit)  {m_n_klm_hit = Nklm_hit;}
    /**set the value of bhabha*/
    void setBhabhaVeto(int bha)  {m_bhabhaveto = bha;}
    /**set the value of sbhabha*/
    void seteclBhabhaVeto(int eclbha)  {m_eclbhabhaveto = eclbha;}
    /**set PairHigh*/
    void setPairHigh(int PhiPairHigh) {m_n_PhiPairHigh = PhiPairHigh;}
    /**set PairLow*/
    void setPairLow(int PhiPairLow) {m_n_PhiPairLow = PhiPairLow;}
    /**set 3DPair*/
    void set3DPair(int n3DPair) {m_n_3DPair = n3DPair;}
    /**set NSameHem1Trk*/
    void setNSameHem1Trk(int SameHem1Trk) {m_n_SameHem1Trk = SameHem1Trk;}
    /**set NOppHem1Trk*/
    void setNOppHem1Trk(int OppHem1Trk) {m_n_OppHem1Trk = OppHem1Trk;}
    /**set the value of Trk_b2b_1to3*/
    void setTrk_b2b_1to3(int Trk_b2b_1to3) {m_Trk_b2b_1to3 = Trk_b2b_1to3;}
    /**set the value of Trk_b2b_1to5*/
    void setTrk_b2b_1to5(int Trk_b2b_1to5) {m_Trk_b2b_1to5 = Trk_b2b_1to5;}
    /**set the value of Trk_b2b_1to7*/
    void setTrk_b2b_1to7(int Trk_b2b_1to7) {m_Trk_b2b_1to7 = Trk_b2b_1to7;}
    /**set the value of Trk_b2b_1to9*/
    void setTrk_b2b_1to9(int Trk_b2b_1to9) {m_Trk_b2b_1to9 = Trk_b2b_1to9;}
    /**set the value of Trk_open90*/
    void setTrk_open90(int Trk_open90) {m_Trk_open90 = Trk_open90;}
    /**set the value of Trk_open30*/
    void setTrk_open30(int Trk_open30) {m_Trk_open30 = Trk_open30;}
    /**set the value of cluster_b2b_1to3*/
    void setcluster_b2b_1to3(int cluster_b2b_1to3) {m_cluster_b2b_1to3 = cluster_b2b_1to3;}
    /**set the value of cluster_b2b_1to5*/
    void setcluster_b2b_1to5(int cluster_b2b_1to5) {m_cluster_b2b_1to5 = cluster_b2b_1to5;}
    /**set the value of cluster_b2b_1to7*/
    void setcluster_b2b_1to7(int cluster_b2b_1to7) {m_cluster_b2b_1to7 = cluster_b2b_1to7;}
    /**set the value of cluster_b2b_1to9*/
    void setcluster_b2b_1to9(int cluster_b2b_1to9) {m_cluster_b2b_1to9 = cluster_b2b_1to9;}
    /**set the value of Trkcluster_b2b_1to3*/
    void setTrkcluster_b2b_1to3(int Trkcluster_b2b_1to3) {m_Trkcluster_b2b_1to3 = Trkcluster_b2b_1to3;}
    /**set the value of Trkcluster_b2b_1to5*/
    void setTrkcluster_b2b_1to5(int Trkcluster_b2b_1to5) {m_Trkcluster_b2b_1to5 = Trkcluster_b2b_1to5;}
    /**set the value of Trkcluster_b2b_1to7*/
    void setTrkcluster_b2b_1to7(int Trkcluster_b2b_1to7) {m_Trkcluster_b2b_1to7 = Trkcluster_b2b_1to7;}
    /**set the value of Trkcluster_b2b_1to9*/
    void setTrkcluster_b2b_1to9(int Trkcluster_b2b_1to9) {m_Trkcluster_b2b_1to9 = Trkcluster_b2b_1to9;}
    /**set eed: two matched & cluster b2b*/
    void seteed(int eed) {m_eed = eed;}
    /**set fed: one track & one matched & cluster b2b*/
    void setfed(int fed) {m_fed = fed;}
    /**set fp: one track & track-cluster b2b*/
    void setfp(int fp) {m_fp = fp;}
    /**set eeb: two matched & track b2b*/
    void seteeb(int eeb) {m_eeb = eeb;}
    /**set fep: one track & one matched & track-cluster b2b*/
    void setfep(int fep) {m_fep = fep;}
    /**set the number of short tracks*/
    void setNshorttrk(int Nshorttrk)  {m_n_st = Nshorttrk;}
    /**set the number of fwd short tracks*/
    void setNshorttrk_fwd(int Nshorttrk_fwd)  {m_n_st_fwd = Nshorttrk_fwd;}
    /**set the number of bwd short tracks*/
    void setNshorttrk_bwd(int Nshorttrk_bwd)  {m_n_st_bwd = Nshorttrk_bwd;}
    /**set s2s3: short-to-short b2b, 10 to 30 degress*/
    void sets2s3(int s2s3) {m_s2s3 = s2s3;}
    /**set s2s5: short-to-short b2b, 10 to 50 degress*/
    void sets2s5(int s2s5) {m_s2s5 = s2s5;}
    /**set s2so: short-to-short opening angle > 90 degrees*/
    void sets2so(int s2so) {m_s2so = s2so;}
    /**set s2s30: short-to-short opening angle > 30 degrees*/
    void sets2s30(int s2s30) {m_s2s30 = s2s30;}
    /**set s2f3: short-to-full b2b, 10 to 30 degress*/
    void sets2f3(int s2f3) {m_s2f3 = s2f3;}
    /**set s2f5: short-to-full b2b, 10 to 50 degress*/
    void sets2f5(int s2f5) {m_s2f5 = s2f5;}
    /**set s2fo: short-to-full opening angle > 90 degrees*/
    void sets2fo(int s2fo) {m_s2fo = s2fo;}
    /**set s2f30: short-to-full opening angle > 30 degrees*/
    void sets2f30(int s2f30) {m_s2f30 = s2f30;}
    /**set bwdsb: short track matched to bhabha cluster in bwd endcap*/
    void setbwdsb(int bwdsb) {m_bwdsb = bwdsb;}
    /**set bwdnb: neutral bhabha cluster in bwd endcap*/
    void setbwdnb(int bwdnb) {m_bwdnb = bwdnb;}
    /**set fwdsb: short track matched to bhabha cluster in fwd endcap*/
    void setfwdsb(int fwdsb) {m_fwdsb = fwdsb;}
    /**set fwdnb: neutral bhabha cluster in fwd endcap*/
    void setfwdnb(int fwdnb) {m_fwdnb = fwdnb;}
    /**set brlfb: full track matched to bhabha cluster in barrel*/
    void setbrlfb(int brlfb) {m_brlfb = brlfb;}
    /**set brlnb: neutral bhabha cluster in barrel*/
    void setbrlnb(int brlnb) {m_brlnb = brlnb;}
    /**set the number of ecl matched short tracks*/
    void setNsecl(int Nsecl)  {m_n_secl = Nsecl;}
    /**set the number of ecl matched fwd short tracks*/
    void setNsecl_fwd(int Nsecl_fwd)  {m_n_secl_fwd = Nsecl_fwd;}
    /**set the number of ecl matched bwd short tracks*/
    void setNsecl_bwd(int Nsecl_bwd)  {m_n_secl_bwd = Nsecl_bwd;}
    /**set the number of klm matched short tracks*/
    void setNsklm(int Nsklm)  {m_n_sklm = Nsklm;}
    /**set the number of klm matched fwd short tracks*/
    void setNsklm_fwd(int Nsklm_fwd)  {m_n_sklm_fwd = Nsklm_fwd;}
    /**set the number of klm matched bwd short tracks*/
    void setNsklm_bwd(int Nsklm_bwd)  {m_n_sklm_bwd = Nsklm_bwd;}
    /**set the number of inner tracks*/
    void setNinnertrk(int Ninnertrk)  {m_n_it = Ninnertrk;}
    /**set i2fo: inner-to-full opening angle > 90 degrees*/
    void seti2fo(int i2fo) {m_i2fo = i2fo;}
    /**set i2io: inner-to-inner opening angle > 90 degrees*/
    void seti2io(int i2io) {m_i2io = i2io;}
    /**set the number of ecl matched inner tracks*/
    void setNiecl(int Niecl)  {m_n_iecl = Niecl;}
    /**set the number of klm matched inner tracks*/
    void setNiklm(int Niklm)  {m_n_iklm = Niklm;}
    /**set GDL input bit*/
    void setNeecleklm(int Nieecleklm)  {m_n_eecleklm = Nieecleklm;}
    /**set GDL input bit*/
    void setInputBits(int i, bool bit) {m_InputBits[i] = bit;}


  private:
    /**#2D finder tracks*/
    int m_n_2dfinder_track;
    /**#2D fitter tracks*/
    int m_n_2dfitter_track;
    /**#3D fitter tracks*/
    int m_n_3dfitter_track;
    /**#3D fitter tracks*/
    int m_n_3dfitter_track_Z10;
    /**#3D fitter tracks*/
    int m_n_3dfitter_track_Z25;
    /**#3D fitter tracks*/
    int m_n_3dfitter_track_Z35;
    /**#NN tracks*/
    int m_n_NN_track;
    /**#NN tracks Z0<20cm*/
    int m_n_NN_track_Z20;
    /**#NN tracks Z0<40cm*/
    int m_n_NN_track_Z40;
    /**#NN tracks Z0<15cm,P>0.7GeV*/
    int m_n_NN_track_STT;
    /**#2D matched finder tracks*/
    int m_n_2dmatch_track;
    /**#3D matched NN tracks*/
    int m_n_3dmatch_track;
    /**#3D NN tracks*/
    //int m_n_NNmatch_track;
    /**# ecl clusers (default energy threshold ~100Mev) */
    int m_n_cluster;
    /**# ecl clusers without associated track */
    int m_n_neutral_cluster;
    /**# ecl clusers with energy threshold 300Mev */
    int m_n_high_300_cluster;
    /**# ecl clusers with energy threshold 1GeV, TC ID 1,2,17 is exlcuded for dedicated single photon trigger */
    int m_n_high_1000_cluster;
    /**# ecl clusers with energy threshold 1GeV */
    int m_n_high_1000_cluster415;
    /**# ecl clusers with energy threshold 1GeV */
    int m_n_high_1000_cluster2316;
    /**# ecl clusers with energy threshold 1GeV */
    int m_n_high_1000_cluster117;
    /**# ecl clusers with energy threshold 2GeV */
    int m_n_high_2000_cluster;
    /**# ecl clusers with energy threshold 2GeV */
    int m_n_high_2000_cluster414;
    /**# ecl clusers with energy threshold 2GeV */
    int m_n_high_2000_cluster231516;
    /**# ecl clusers with energy threshold 2GeV */
    int m_n_high_2000_cluster117;
    /**# ecl clusers with energy threshold 2GeV in TC ID 1 and 17 */
    int m_n_high_2000_endcap_cluster;
    /**# back to back ecl cluser pairs */
    int m_nbbc;
    /**# back to back cdc track and ecl clsuter*/
    int m_nbbtc;
    /**# klm track*/
    int m_n_klm_track;
    /**# klm hit*/
    int m_n_klm_hit;
    /**two track Bhabha veto, bahbah:1, non-bhabha:0*/
    int m_bhabhaveto;
    /**ecl Bhabha veto (only ecl information is used), eclbahbah:1, non-eclbhabha:0*/
    int m_eclbhabhaveto;
    /**number of PhiPairHigh*/
    int m_n_PhiPairHigh;
    /**number of PhiPairLow*/
    int m_n_PhiPairLow;
    /**number of 3DPair*/
    int m_n_3DPair;
    /**number of SameHem+1Trk*/
    int m_n_SameHem1Trk;
    /**number of OppHem+1Trk*/
    int m_n_OppHem1Trk;
    /**Trk_b2b_1to3*/
    int m_Trk_b2b_1to3;
    /**Trk_b2b_1to5*/
    int m_Trk_b2b_1to5;
    /**Trk_b2b_1to7*/
    int m_Trk_b2b_1to7;
    /**Trk_b2b_1to9*/
    int m_Trk_b2b_1to9;
    /**Trk_open90*/
    int m_Trk_open90;
    /**Trk_open30*/
    int m_Trk_open30;
    /**cluster_b2b_1to3*/
    int m_cluster_b2b_1to3;
    /**cluster_b2b_1to5*/
    int m_cluster_b2b_1to5;
    /**cluster_b2b_1to7*/
    int m_cluster_b2b_1to7;
    /**cluster_b2b_1to9*/
    int m_cluster_b2b_1to9;
    /**Trkcluster_b2b_1to3*/
    int m_Trkcluster_b2b_1to3;
    /**Trkcluster_b2b_1to5*/
    int m_Trkcluster_b2b_1to5;
    /**Trkcluster_b2b_1to7*/
    int m_Trkcluster_b2b_1to7;
    /**Trkcluster_b2b_1to9*/
    int m_Trkcluster_b2b_1to9;
    /**eed: two matched & cluster b2b*/
    int m_eed;
    /**fed: one track & one matched & cluster b2b*/
    int m_fed;
    /**fp: one track & track-cluster b2b*/
    int m_fp;
    /**eeb: two matched & track b2b*/
    int m_eeb;
    /**fep: one track & one matched & track-cluster b2b*/
    int m_fep;
    /**the number of short tracks*/
    int m_n_st;
    /**the number of fwd short tracks*/
    int m_n_st_fwd;
    /**the number of bwd short tracks*/
    int m_n_st_bwd;
    /**s2s3: short-to-short b2b, 10 to 30 degress*/
    int m_s2s3;
    /**s2s5: short-to-short b2b, 10 to 50 degress*/
    int m_s2s5;
    /**s2so: short-to-short opening angle > 90 degrees*/
    int m_s2so;
    /**s2s30: short-to-short opening angle > 30 degrees*/
    int m_s2s30;
    /**s2f3: short-to-full b2b, 10 to 30 degress*/
    int m_s2f3;
    /**s2f5: short-to-full b2b, 10 to 50 degress*/
    int m_s2f5;
    /**s2fo: short-to-full opening angle > 90 degrees*/
    int m_s2fo;
    /**s2f30: short-to-full opening angle > 30 degrees*/
    int m_s2f30;
    /**bwdsb: short track matched to bhabha cluster in bwd endcap*/
    int m_bwdsb;
    /**bwdnb: neutral bhabha cluster in bwd endcap*/
    int m_bwdnb;
    /**fwdsb: short track matched to bhabha cluster in fwd endcap*/
    int m_fwdsb;
    /**fwdnb: neutral bhabha cluster in fwd endcap*/
    int m_fwdnb;
    /**brlfb: full track matched to bhabha cluster in barrel*/
    int m_brlfb;
    /**brlnb: neutral bhabha cluster in barrel*/
    int m_brlnb;
    /**the number of ecl matched short tracks*/
    int m_n_secl;
    /**the number of ecl matched fwd short tracks*/
    int m_n_secl_fwd;
    /**the number of ecl matched bwd short tracks*/
    int m_n_secl_bwd;
    /**the number of klm matched short tracks*/
    int m_n_sklm;
    /**the number of klm matched fwd short tracks*/
    int m_n_sklm_fwd;
    /**the number of klm matched bwd short tracks*/
    int m_n_sklm_bwd;
    /**the number of inner tracks*/
    int m_n_it;
    /**i2fo: inner-to-full opening angle > 90 degrees*/
    int m_i2fo;
    /**i2io: inner-to-inner opening angle > 90 degrees*/
    int m_i2io;
    /**the number of ecl matched inner tracks*/
    int m_n_iecl;
    /**the number of klm matched inner tracks*/
    int m_n_iklm;
    /**the number of klm matched ecl clusters at endcap*/
    int m_n_eecleklm;
    /**GDL input bits**/
    std::vector<bool> m_InputBits = std::vector<bool>(320, false);

    /**! The Class title*/
    ClassDef(TRGGRLInfo, 7); /*< the class title */
  };
} // end namespace Belle2

//#endif
