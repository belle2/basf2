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
#include <TVector3.h>
namespace Belle2 {

  class TRGGRLInfo : public TObject {
  public:

    TRGGRLInfo():
      m_n_2dfinder_track(-1),
      m_n_2dfitter_track(-1),
      m_n_3dfitter_track(-1),
      m_n_3dfitter_track_Z10(-1),
      m_n_3dfitter_track_Z25(-1),
      m_n_NN_track(-1),
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
      m_fp(-1)
    {}
    ~TRGGRLInfo() {}


    /**set the number of 2D tracks*/
    int getN2Dfindertrk() const  {return m_n_2dfinder_track;}
    /**get the number of 2D fitter tracks*/
    int getN2Dfittertrk() const  {return m_n_2dfitter_track;}
    /**get the number of 3D charged tracks*/
    int getN3Dfittertrk() const  {return m_n_3dfitter_track;}
    /**get the number of 3D charged tracks*/
    int getN3DfittertrkZ10() const  {return m_n_3dfitter_track_Z10;}
    /**get the number of 3D charged tracks*/
    int getN3DfittertrkZ25() const  {return m_n_3dfitter_track_Z25;}
    /**get the number of NN charged tracks*/
    int getNNNtrk() const  {return m_n_NN_track;}
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
    /***/
    int getPhiPairHigh() const {return m_n_PhiPairHigh;}
    /***/
    int getPhiPairLow() const {return m_n_PhiPairLow;}
    /***/
    int get3DPair() const {return m_n_3DPair;}
    /***/
    int getNSameHem1Trk() const {return m_n_SameHem1Trk;}
    /***/
    int getNOppHem1Trk() const {return m_n_OppHem1Trk;}
    /**get Trk_b2b_1to3*/
    int getTrk_b2b_1to3() const {return m_Trk_b2b_1to3;}
    /**get Trk_b2b_1to5*/
    int getTrk_b2b_1to5() const {return m_Trk_b2b_1to5;}
    /**get Trk_b2b_1to7*/
    int getTrk_b2b_1to7() const {return m_Trk_b2b_1to7;}
    /**get Trk_b2b_1to9*/
    int getTrk_b2b_1to9() const {return m_Trk_b2b_1to9;}
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
    /**set the number of NN charged tracks*/
    void setNNNtrk(int NNNtrk)  {m_n_NN_track = NNNtrk;}
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
    /***/
    void setPairHigh(int PhiPairHigh) {m_n_PhiPairHigh = PhiPairHigh;}
    /***/
    void setPairLow(int PhiPairLow) {m_n_PhiPairLow = PhiPairLow;}
    /***/
    void set3DPair(int n3DPair) {m_n_3DPair = n3DPair;}
    /***/
    void setNSameHem1Trk(int SameHem1Trk) {m_n_SameHem1Trk = SameHem1Trk;}
    /***/
    void setNOppHem1Trk(int OppHem1Trk) {m_n_OppHem1Trk = OppHem1Trk;}
    /**set the value of Trk_b2b_1to3*/
    void setTrk_b2b_1to3(int Trk_b2b_1to3) {m_Trk_b2b_1to3 = Trk_b2b_1to3;}
    /**set the value of Trk_b2b_1to5*/
    void setTrk_b2b_1to5(int Trk_b2b_1to5) {m_Trk_b2b_1to5 = Trk_b2b_1to5;}
    /**set the value of Trk_b2b_1to7*/
    void setTrk_b2b_1to7(int Trk_b2b_1to7) {m_Trk_b2b_1to7 = Trk_b2b_1to7;}
    /**set the value of Trk_b2b_1to9*/
    void setTrk_b2b_1to9(int Trk_b2b_1to9) {m_Trk_b2b_1to9 = Trk_b2b_1to9;}
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
    /**set fp: one track & trackk-cluster b2b*/
    void setfp(int fp) {m_fp = fp;}


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
    /**#NN tracks*/
    int m_n_NN_track;
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
    /***/
    int m_n_PhiPairHigh;
    /***/
    int m_n_PhiPairLow;
    /***/
    int m_n_3DPair;
    /***/
    int m_n_SameHem1Trk;
    /***/
    int m_n_OppHem1Trk;
    /**Trk_b2b_1to3*/
    int m_Trk_b2b_1to3;
    /**Trk_b2b_1to5*/
    int m_Trk_b2b_1to5;
    /**Trk_b2b_1to7*/
    int m_Trk_b2b_1to7;
    /**Trk_b2b_1to9*/
    int m_Trk_b2b_1to9;
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

    /**! The Class title*/
    ClassDef(TRGGRLInfo, 2); /*< the class title */
  };
} // end namespace Belle2

//#endif
