//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLDigi0MC.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLDIGI0MC_H
#define TRGECLDIGI0MC_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLDigi0MC : public TObject {
  public:

    // Event Id
    int m_eventid;
    // TC id
    int m_tcid;
    int m_ibintime;
    // TC Energy and Timing
    double m_raw_energy;
    double m_raw_timing;
    double m_signal_contribution;
    double m_bkg_contribution;


    //MC Infomation
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


    // Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    // Set TC id
    void setTCId(int tcid) { m_tcid = tcid; }
    //
    void setiBinTime(int ibintime) { m_ibintime = ibintime; }

    // Set Energy and Timing
    void setRawEnergy(double raw_energy) { m_raw_energy = raw_energy; }
    void setSignalContribution(double signal_contribution) { m_signal_contribution =  signal_contribution; }
    void setBKGContribution(double bkg_contribution) { m_bkg_contribution =  bkg_contribution; }

    void setRawTiming(double raw_timing) { m_raw_timing = raw_timing; }
    //
    //MC information
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












    // Get event id
    int getEventId() const { return m_eventid; }
    // Get TC id
    int getTCId() const { return m_tcid; }
    //
    int getiBinTime() const { return m_ibintime; }
    // Get Energy and Timing
    double getRawEnergy() const {return m_raw_energy; }
    double getRawTiming() const {return m_raw_timing; }
    //MC information
    /* int getTrackId(int i) const{return m_trackId[i] ; } */
    /* int getPDG(int i) const{ return m_pdg[i] ; } */
    /* int getMother(int i) const{return m_mother[i] ; } */
    /* int getGMother(int i) const{return m_gmother[i] ; } */
    /* int getGGMother(int i) const{return m_ggmother[i] ; } */
    /* int getGGGMother(int i) const{return m_gggmother[i] ; } */

    /* int getBackgroundTag(int i)const {return m_background_tag[i] ; } */

    /* double getPX(int i) const{return m_momentum_x[i]; } */
    /* double getPY(int i)const {return m_momentum_y[i] ; } */
    /* double getPZ(int i)const {return m_momentum_z[i] ; } */
    /* double getEnergy(int i)const {return m_energy[i] ; } */

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLDigi0MC() {;}

    // Useful Constructor
    TRGECLDigi0MC(
      int eventid,
      int tcid,
      int ibintime,
      double raw_energy,
      double raw_timing
    ) {
      m_eventid = eventid;
      m_tcid    = tcid;
      m_ibintime = ibintime;
      m_raw_energy = raw_energy;
      m_raw_timing = raw_timing;
    }
    ClassDef(TRGECLDigi0MC, 1); /*< the class title */
  };
} // end namespace Belle2

#endif
