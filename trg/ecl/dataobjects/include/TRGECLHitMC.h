//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLHit.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGECLHITMC_H
#define TRGECLHITMC_H


#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLHitMC : public TObject {
  public:


    //! The cell id of this hit.
    int m_eventId;

    //! The cell id of this hit.
    int m_cellId;

    //! The # of output of TC.
    int m_noutput;


    //! Deposited energy of this hit.
    double m_edep;

    //! Hit average time
    double m_TimeAve;

    //MC Infomation
    /* int m_background_tag; */
    /* int m_trackId; */
    /* int m_pdg; */
    /* int m_mother; */
    /* int m_gmother; */
    /* int m_ggmother; */
    /* int m_gggmother; */

    /* double m_momentum_x; */
    /* double m_momentum_y; */
    /* double m_momentum_z; */
    /* double m_energy; */
    int m_cellId_1;
    int m_background_tag_1;
    int m_trackId_1;
    int m_pdg_1;
    int m_mother_1;
    int m_gmother_1;
    int m_ggmother_1;
    int m_gggmother_1;
    int m_ggggmother_1;

    double m_momentum_x_1;
    double m_momentum_y_1;
    double m_momentum_z_1;
    double m_energy_1;
    double m_contribution_1;

    int m_cellId_2;
    int m_background_tag_2;
    int m_trackId_2;
    int m_pdg_2;
    int m_mother_2;
    int m_gmother_2;
    int m_ggmother_2;
    int m_gggmother_2;
    int m_ggggmother_2;

    double m_momentum_x_2;
    double m_momentum_y_2;
    double m_momentum_z_2;
    double m_energy_2;
    double m_contribution_2;

    int m_cellId_3;
    int m_background_tag_3;
    int m_trackId_3;
    int m_pdg_3;
    int m_mother_3;
    int m_gmother_3;
    int m_ggmother_3;
    int m_gggmother_3;
    int m_ggggmother_3;

    double m_momentum_x_3;
    double m_momentum_y_3;
    double m_momentum_z_3;
    double m_energy_3;
    double m_contribution_3;

    double m_signal_contribution;
    double m_bkg_contribution;


    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId; }

    //! The method to set cell id
    void setTCId(int cellId) { m_cellId = cellId; }

    //! The method to set # of output per TC
    void setTCOutput(int noutput) { m_noutput = noutput; }
    void setSignalContribution(double signal_contribution) { m_signal_contribution =  signal_contribution; }

    void setBKGContribution(double bkg_contribution) { m_bkg_contribution =  bkg_contribution; }


    //MC information
    /* void setTrackId(int track) { m_trackId = track; } */
    /* void setPDG(int pdg) { m_pdg = pdg; } */
    /* void setMother(int mother) { m_mother = mother; } */
    /* void setGMother(int gmother) { m_gmother = gmother; } */
    /* void setGGMother(int ggmother) { m_ggmother = ggmother; } */
    /* void setGGGMother(int gggmother) { m_gggmother = gggmother; } */


    /* void setBackgroundTag(int back) { m_background_tag = back; } */

    /* void setPX(double px) { m_momentum_x = px; } */
    /* void setPY(double py) { m_momentum_y = py; } */
    /* void setPZ(double pz) { m_momentum_z = pz; } */
    /* void setMCEnergy(double energy) { m_energy = energy; } */
    void setCellId(int cell[3]) {
      m_cellId_1 = cell[0];
      m_cellId_2 = cell[1];
      m_cellId_3 = cell[2];

    }
    void setTrackId(int track[3]) {
      m_trackId_1 = track[0];
      m_trackId_2 = track[1];
      m_trackId_3 = track[2];

    }
    void setPDG(int pdg[3]) {
      m_pdg_1 = pdg[0];
      m_pdg_2 = pdg[1];
      m_pdg_3 = pdg[2];
    }

    void setMother(int mother[3]) {
      m_mother_1 = mother[0];
      m_mother_2 = mother[1];
      m_mother_3 = mother[2];
    }
    void setGMother(int gmother[3]) {
      m_gmother_1 = gmother[0];
      m_gmother_2 = gmother[1];
      m_gmother_3 = gmother[2];
    }
    void setGGMother(int ggmother[3]) {
      m_ggmother_1 = ggmother[0];
      m_ggmother_2 = ggmother[1];
      m_ggmother_3 = ggmother[2];
    }
    void setGGGMother(int gggmother[3]) {
      m_gggmother_1 = gggmother[0];
      m_gggmother_2 = gggmother[1];
      m_gggmother_3 = gggmother[2];
    }

    void setBackgroundTag(int back[3]) {
      m_background_tag_1 = back[0];
      m_background_tag_2 = back[1];
      m_background_tag_3 = back[2];

    }

    void setPX(double px[3]) {
      m_momentum_x_1 = px[0];
      m_momentum_x_2 = px[1];
      m_momentum_x_3 = px[2];
    }
    void setPY(double py[3]) {
      m_momentum_y_1 = py[0];
      m_momentum_y_2 = py[1];
      m_momentum_y_3 = py[2];
    }
    void setPZ(double pz[3]) {
      m_momentum_z_1 = pz[0];
      m_momentum_z_2 = pz[1];
      m_momentum_z_3 = pz[2];
    }

    void setMCEnergy(double energy[3]) {
      m_energy_1 = energy[0];
      m_energy_2 = energy[1];
      m_energy_3 = energy[2];

    }
    void setContribution(double contribution[3]) {
      m_contribution_1 = contribution[0];
      m_contribution_2 = contribution[1];
      m_contribution_3 = contribution[1];
    }





    //! The method to set deposited energy
    void setEnergyDep(double edep) { m_edep = edep; }

    //! The method to set hit average time
    void setTimeAve(double TimeAve) { m_TimeAve = TimeAve; }

    //! The method to get event id
    int getEventId() const { return m_eventId; }

    //! The method to get cell id
    int getCellId() const { return m_cellId; }

    //! The method to get deposited energy
    double getEnergyDep() const { return m_edep; }

    //! The method to get hit average time
    double getTimeAve() const {return m_TimeAve;}
    //MC information
    /* int getTrackId() const{return m_trackId ; } */
    /* int getPDG() const{ return m_pdg ; } */
    /* int getMother() const{return m_mother ; } */
    /* int getGMother() const{return m_gmother ; } */
    /* int getGGMother() const{return m_ggmother ; } */
    /* int getGGGMother() const{return m_gggmother ; } */

    /* int getBackgroundTag()const {return m_background_tag ; } */

    /* double getPX() const{return m_momentum_x ; } */
    /* double getPY()const {return m_momentum_y ; } */
    /* double getPZ()const {return m_momentum_z ; } */
    /* double getEnergy()const {return m_energy ; } */



    // Empty constructor
    // Recommended for ROOT IO
    TRGECLHitMC() {;}

    //! Useful Constructor
    TRGECLHitMC(
      int eventId,
      int cellId,
      int noutput,
      double edep,
      double TimeAve
    ) {
      m_eventId = eventId;
      m_cellId = cellId;
      m_edep = edep;
      m_TimeAve = TimeAve;
      m_noutput = noutput;
    }
    ClassDef(TRGECLHitMC, 1); /*< the class title */

  };

} // end namespace Belle2

#endif
