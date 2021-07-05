/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLDIGI0MC_H
#define TRGECLDIGI0MC_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLDigi0MC : public TObject {
  public:

    //! Event Id
    int m_eventid;
    //! TC id
    int m_tcid;
    //! time bin
    int m_ibintime;
    //! TC Energy and Timing
    //! raw TC energy
    double m_raw_energy;
    //! raw TC timing
    double m_raw_timing;
    //! Signal contribution
    double m_signal_contribution;
    //! beam background contribution
    double m_bkg_contribution;



    //! 1st contribution particle information

    //! TCID
    int m_cellId_1;
    //! beam background tag
    int m_background_tag_1;
    //! track ID
    int m_trackId_1;
    //! PDF
    int m_pdg_1;
    //! Mother ID
    int m_mother_1;
    //! Grand mother ID
    int m_gmother_1;
    //! Grand Grand Mother ID
    int m_ggmother_1;
    //! Grand Grand Grand Mother ID
    int m_gggmother_1;
    //! Grand grand grand grand Mother ID
    int m_ggggmother_1;

    //! momentum x
    double m_momentum_x_1;
    //! momentum y
    double m_momentum_y_1;
    //! momentum z
    double m_momentum_z_1;
    //! energy
    double m_energy_1;
    //! contribution
    double m_contribution_1;

    //! 2nd contribution particle information

    //! TCID
    int m_cellId_2;
    //! beam background tag
    int m_background_tag_2;
    //! track ID
    int m_trackId_2;
    //! PDF
    int m_pdg_2;
    //! Mother ID
    int m_mother_2;
    //! Grand mother ID
    int m_gmother_2;
    //! Grand Grand Mother ID
    int m_ggmother_2;
    //! Grand Grand Grand Mother ID
    int m_gggmother_2;
    //! Grand grand grand grand Mother ID
    int m_ggggmother_2;

    //! momentum x
    double m_momentum_x_2;
    //! momentum y
    double m_momentum_y_2;
    //! momentum z
    double m_momentum_z_2;
    //! energy
    double m_energy_2;
    //! contribution
    double m_contribution_2;

    //! 3rd contribution particle information

    //! TCID
    int m_cellId_3;
    //! beam background tag
    int m_background_tag_3;
    //! track ID
    int m_trackId_3;
    //! PDF
    int m_pdg_3;
    //! Mother ID
    int m_mother_3;
    //! Grand mother ID
    int m_gmother_3;
    //! Grand Grand Mother ID
    int m_ggmother_3;
    //! Grand Grand Grand Mother ID
    int m_gggmother_3;
    //! Grand grand grand grand Mother ID
    int m_ggggmother_3;

    //! momentum x
    double m_momentum_x_3;
    //! momentum y
    double m_momentum_y_3;
    //! momentum z
    double m_momentum_z_3;
    //! energy
    double m_energy_3;
    //! contribution
    double m_contribution_3;


    //! Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    //! Set TC id
    void setTCId(int tcid) { m_tcid = tcid; }
    //! Set Bin Number
    void setiBinTime(int ibintime) { m_ibintime = ibintime; }

    //! Set Energy and Timing
    void setRawEnergy(double raw_energy) { m_raw_energy = raw_energy; }
    //! set Signal Contribtion
    void setSignalContribution(double signal_contribution) { m_signal_contribution =  signal_contribution; }
    //! set beambackground contribution
    void setBKGContribution(double bkg_contribution) { m_bkg_contribution =  bkg_contribution; }
    //! set Raw timing
    void setRawTiming(double raw_timing) { m_raw_timing = raw_timing; }
    //!
    //!MC information


//! set TC ID
    void setCellId(int cell[3])
    {
      m_cellId_1 = cell[0];
      m_cellId_2 = cell[1];
      m_cellId_3 = cell[2];

    }
    //! set Track Id
    void setTrackId(int track[3])
    {
      m_trackId_1 = track[0];
      m_trackId_2 = track[1];
      m_trackId_3 = track[2];

    }
    //! set PDG
    void setPDG(int pdg[3])
    {
      m_pdg_1 = pdg[0];
      m_pdg_2 = pdg[1];
      m_pdg_3 = pdg[2];
    }
    //! set Mother
    void setMother(int mother[3])
    {
      m_mother_1 = mother[0];
      m_mother_2 = mother[1];
      m_mother_3 = mother[2];
    }
    //! set Grand Mother
    void setGMother(int gmother[3])
    {
      m_gmother_1 = gmother[0];
      m_gmother_2 = gmother[1];
      m_gmother_3 = gmother[2];
    }
    //! set Grand Grand Mother
    void setGGMother(int ggmother[3])
    {
      m_ggmother_1 = ggmother[0];
      m_ggmother_2 = ggmother[1];
      m_ggmother_3 = ggmother[2];
    }
    //! set Grand Grand grandMother
    void setGGGMother(int gggmother[3])
    {
      m_gggmother_1 = gggmother[0];
      m_gggmother_2 = gggmother[1];
      m_gggmother_3 = gggmother[2];
    }
    //! set grand grand grand grand Mother
    void setBackgroundTag(int back[3])
    {
      m_background_tag_1 = back[0];
      m_background_tag_2 = back[1];
      m_background_tag_3 = back[2];

    }
    //! set momentum X
    void setPX(double px[3])
    {
      m_momentum_x_1 = px[0];
      m_momentum_x_2 = px[1];
      m_momentum_x_3 = px[2];
    }
    //! set momentum Y
    void setPY(double py[3])
    {
      m_momentum_y_1 = py[0];
      m_momentum_y_2 = py[1];
      m_momentum_y_3 = py[2];
    }
    //! set momentum Z
    void setPZ(double pz[3])
    {
      m_momentum_z_1 = pz[0];
      m_momentum_z_2 = pz[1];
      m_momentum_z_3 = pz[2];
    }
    //! set energy
    void setMCEnergy(double energy[3])
    {
      m_energy_1 = energy[0];
      m_energy_2 = energy[1];
      m_energy_3 = energy[2];

    }
    //! set contribution
    void setContribution(double contribution[3])
    {
      m_contribution_1 = contribution[0];
      m_contribution_2 = contribution[1];
      m_contribution_3 = contribution[1];
    }










    //! Get event id
    int getEventId() const { return m_eventid; }
    //! Get TC id
    int getTCId() const { return m_tcid; }
    //! get Bin number
    int getiBinTime() const { return m_ibintime; }
    //! Get Raw Energy
    double getRawEnergy() const {return m_raw_energy; }
    //! Get Raw Timing
    double getRawTiming() const {return m_raw_timing; }

    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLDigi0MC()
    {
      m_eventid   = 0;
      m_tcid      = 0;
      m_ibintime  = 0;
      m_raw_energy = 0;
      m_raw_timing = 0;


      m_background_tag_1 = 0;
      m_background_tag_2 = 0;
      m_background_tag_3 = 0;
      m_bkg_contribution = 0;
      m_signal_contribution = 0;
      m_cellId_1 = 0;
      m_cellId_2 = 0;
      m_cellId_3 = 0;
      m_contribution_1 = 0;
      m_contribution_2 = 0;
      m_contribution_3 = 0;
      m_energy_1 = 0;
      m_energy_2 = 0;
      m_energy_3 = 0;
      m_ggggmother_1 = 0;
      m_ggggmother_2 = 0;
      m_ggggmother_3 = 0;
      m_gggmother_1 = 0;
      m_gggmother_2 = 0;
      m_gggmother_3 = 0;
      m_ggmother_1 = 0;
      m_ggmother_2 = 0;
      m_ggmother_3 = 0;
      m_gmother_1 = 0;
      m_gmother_2 = 0;
      m_gmother_3 = 0;
      m_mother_1 = 0;
      m_mother_2 = 0;
      m_mother_3 = 0;


      m_momentum_x_1 = 0;
      m_momentum_x_2 = 0;
      m_momentum_x_3 = 0;
      m_momentum_y_1 = 0;
      m_momentum_y_2 = 0;
      m_momentum_y_3 = 0;
      m_momentum_z_1 = 0;
      m_momentum_z_2 = 0;
      m_momentum_z_3 = 0;

      m_pdg_1 = 0;  ;
      m_pdg_2 = 0;  ;
      m_pdg_3 = 0;  ;
      m_trackId_1 = 0;
      m_trackId_2 = 0;
      m_trackId_3 = 0;



    }

    //! Useful Constructor
    TRGECLDigi0MC(
      int eventid,
      int tcid,
      int ibintime,
      double raw_energy,
      double raw_timing,
      int background_tag_1,
      int background_tag_2,
      int background_tag_3,
      int bkg_contribution,
      int signal_contribution,
      int cellId_1,
      int cellId_2,
      int cellId_3,
      int contribution_1,
      int contribution_2,
      int contribution_3,
      int energy_1,
      int energy_2,
      int energy_3,
      int ggggmother_1,
      int ggggmother_2,
      int ggggmother_3,
      int gggmother_1,
      int gggmother_2,
      int gggmother_3,
      int ggmother_1,
      int ggmother_2,
      int ggmother_3,
      int gmother_1,
      int gmother_2,
      int gmother_3,
      int mother_1,
      int mother_2,
      int mother_3,
      int momentum_x_1,
      int momentum_x_2,
      int momentum_x_3,
      int momentum_y_1,
      int momentum_y_2,
      int momentum_y_3,
      int momentum_z_1,
      int momentum_z_2,
      int momentum_z_3,
      int pdg_1,
      int pdg_2,
      int pdg_3,
      int trackId_1,
      int trackId_2,
      int trackId_3
    )
    {
      m_eventid = eventid;
      m_tcid    = tcid;
      m_ibintime = ibintime;
      m_raw_energy = raw_energy;
      m_raw_timing = raw_timing;


      m_background_tag_1 = background_tag_1;
      m_background_tag_2 = background_tag_2;
      m_background_tag_3 = background_tag_3;
      m_bkg_contribution = bkg_contribution;
      m_signal_contribution = signal_contribution;
      m_cellId_1 = cellId_1;
      m_cellId_2 = cellId_2;
      m_cellId_3 = cellId_3;
      m_contribution_1 = contribution_1;
      m_contribution_2 = contribution_2;
      m_contribution_3 = contribution_3;
      m_energy_1 = energy_1;
      m_energy_2 = energy_2;
      m_energy_3 = energy_3;
      m_ggggmother_1 = ggggmother_1;
      m_ggggmother_2 = ggggmother_2;
      m_ggggmother_3 = ggggmother_3;
      m_gggmother_1 = gggmother_1;
      m_gggmother_2 = gggmother_2;
      m_gggmother_3 = gggmother_3;
      m_ggmother_1 = ggmother_1;
      m_ggmother_2 = ggmother_2;
      m_ggmother_3 = ggmother_3;
      m_gmother_1 = gmother_1;
      m_gmother_2 = gmother_2;
      m_gmother_3 = gmother_3;
      m_mother_1 = mother_1;
      m_mother_2 = mother_2;
      m_mother_3 = mother_3;


      m_momentum_x_1 = momentum_x_1;
      m_momentum_x_2 = momentum_x_2;
      m_momentum_x_3 = momentum_x_3;
      m_momentum_y_1 = momentum_y_1;
      m_momentum_y_2 = momentum_y_2;
      m_momentum_y_3 = momentum_y_3;
      m_momentum_z_1 = momentum_z_1;
      m_momentum_z_2 = momentum_z_2;
      m_momentum_z_3 = momentum_z_3;

      m_pdg_1 = pdg_1;
      m_pdg_2 = pdg_2;
      m_pdg_3 = pdg_3;
      m_trackId_1 = trackId_1;
      m_trackId_2 = trackId_2;
      m_trackId_3 = trackId_3;



    }
    //! the class title
    ClassDef(TRGECLDigi0MC, 1); /*< the class title */
  };
} //! end namespace Belle2

#endif
