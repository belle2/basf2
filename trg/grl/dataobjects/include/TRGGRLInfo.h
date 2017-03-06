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

    TRGGRLInfo() {}
    ~TRGGRLInfo() {}


    //set the number of 2D tracks
    int getN2Dfindertrk() const  {return m_n_2dfinder_track;}
    //get the number of 2D fitter tracks
    int getN2Dfittertrk() const  {return m_n_2dfitter_track;}
    //get the number of 3D charged tracks
    int getN3Dfittertrk() const  {return m_n_3dfitter_track;}
    //get the number of NN charged tracks
    int getNNNtrk() const  {return m_n_NN_track;}
    //get the number of 2D matched tracks
    int getN2Dmatchtrk() const {return m_n_2dmatch_track;}
    //get the number of 3D matched tracks
    int getN3Dmatchtrk() const {return m_n_3dmatch_track;}
    //get the number of 3D NN tracks
    //int getNNNmatchtrk() const {return m_n_NNmatch_track;}
    //get the number of eclcluster
    int getNcluster() const {return m_n_cluster;}
    //get the number of eclclusters >300MeV
    int getNhighcluster1() const {return m_n_high_300_cluster;}
    //get the number of eclcluster >1GeV
    int getNhighcluster2() const {return m_n_high_1000_cluster;}
    //get the number of eclclusters without associated tracks
    int getNneucluster() const {return m_n_neutral_cluster;}
    //get the number of klm tracks
    int getNklmtrk() const {return m_n_klm_track;}
    //get the number of klm hits
    int getNklmhit() const {return m_n_klm_hit;}
    //get two track bhabha logic
    int getBhabhaVeto() const {return m_bhabhaveto;}
    //get one track bhabha logic
    int getsBhabhaVeto() const {return m_sbhabhaveto;}
    //get eclbhabha
    int geteclBhabhaVeto() const {return m_eclbhabhaveto;}
    //get bhabha accept logic 1
    int getBhabhaAccept1() const {return m_bhabha_accept1;}
    //get bhabha accept logic 2
    int getBhabhaAccept2() const {return m_bhabha_accept2;}
    //get gg accept logic 1
    int getggAccept1() const {return m_gg_accept1;}
    //get gg accept logic 2
    int getggAccept2() const  {return m_gg_accept2;}
    //get the number of back to back cluster pairs
    int getNbbCluster() const  {return m_nbbc;}

    //set the number of 2D tracks
    void setN2Dfindertrk(int N2Dfindertrk)  {m_n_2dfinder_track = N2Dfindertrk;}
    //set the number of 2D fitter tracks
    void setN2Dfittertrk(int N2Dfittertrk)  {m_n_2dfitter_track = N2Dfittertrk;}
    //set the number of 3D charged tracks
    void setN3Dfittertrk(int N3Dfittertrk)  {m_n_3dfitter_track = N3Dfittertrk;}
    //set the number of NN charged tracks
    void setNNNtrk(int NNNtrk)  {m_n_NN_track = NNNtrk;}
    //set the number of 2D matched tracks
    void setN2Dmatchtrk(int N2Dmatchtrk)  {m_n_2dmatch_track = N2Dmatchtrk;}
    //set the number of 3D matched tracks
    void setN3Dmatchtrk(int N3Dmatchtrk)  {m_n_3dmatch_track = N3Dmatchtrk;}
    //set the number of NN tracks
    //void setNNNmatchtrk(int NNNmatchtrk)  {m_n_NNmatch_track = NNNmatchtrk;}
    //set the number of clusters
    void setNcluster(int Ncluster)  {m_n_cluster = Ncluster;}
    //set the number of cluster >300 MeV
    void setNhighcluster1(int Nclusterh1)  {m_n_high_300_cluster = Nclusterh1;}
    //set the number of cluster >1 GeV
    void setNhighcluster2(int Nclusterh2)  {m_n_high_1000_cluster = Nclusterh2;}
    //set the number of cluster w/o associated tracks
    void setNneucluster(int Nneucluster)  {m_n_neutral_cluster = Nneucluster;}
    //set the number of klm tracks
    void setNklmtrk(int Nklm_track)  {m_n_klm_track = Nklm_track;}
    //set the number of klm hits
    void setNklmhit(int Nklm_hit)  {m_n_klm_hit = Nklm_hit;}
    //set the value of bhabha
    void setBhabhaVeto(int bha)  {m_bhabhaveto = bha;}
    //set the value of sbhabha
    void setsBhabhaVeto(int sbha)  {m_sbhabhaveto = sbha;}
    //set the value of sbhabha
    void seteclBhabhaVeto(int eclbha)  {m_eclbhabhaveto = eclbha;}
    //set the value of bhabha accept1
    void setBhabhaAccept1(int bhaa1)  {m_bhabha_accept1 = bhaa1;}
    //set the value of bhabha accept2
    void setBhabhaAccept2(int bhaa2)  {m_bhabha_accept2 = bhaa2;}
    //set the value of gg accept1
    void setggAccept1(int gga1)  {m_gg_accept1 = gga1;}
    //set the value of gg accept2
    void setggAccept2(int gga2)  {m_gg_accept2 = gga2;}
    //set the number of back to back eclcluster pairs
    void setNbbCluster(int nbbc)  {m_nbbc = nbbc;}


  private:
    /**#2D finder tracks*/
    int m_n_2dfinder_track;
    /**#2D fitter tracks*/
    int m_n_2dfitter_track;
    /**#3D fitter tracks*/
    int m_n_3dfitter_track;
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
    /**# back to back ecl cluser pairs */
    int m_nbbc;
    /**# klm track*/
    int m_n_klm_track;
    /**# klm hit*/
    int m_n_klm_hit;
    /**two track Bhabha veto, bahbah:1, non-bhabha:0*/
    int m_bhabhaveto;
    /**one track Bhabha veto, sbahbah:1, non-sbhabha:0*/
    int m_sbhabhaveto;
    /**ecl Bhabha veto (only ecl information is used), eclbahbah:1, non-eclbhabha:0*/
    int m_eclbhabhaveto;
    /**Bhabha accept, bhabha:1, non-bhabha:0*/
    int m_bhabha_accept1;
    /**Bhabha accept, bhabha:1, non-bhabha:0*/
    int m_bhabha_accept2;
    /**gamma gamma accept, gg:1, non-gg:0*/
    int m_gg_accept1;
    /**gamma gamma accept, gg:1, non-gg:0*/
    int m_gg_accept2;


    //! The Class title
    ClassDef(TRGGRLInfo, 1); /*< the class title */
  };

} // end namespace Belle2

//#endif
