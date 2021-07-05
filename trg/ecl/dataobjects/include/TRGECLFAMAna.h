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
// Filename : TRGECLDigi.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLFAMANA_H
#define TRGECLFAMANA_H

#include <TObject.h>

namespace Belle2 {

  //!  Digitize result
  class TRGECLFAMAna : public TObject {
  public:

    //! Event Id
    int m_eventid;
    //! TC id
    int m_tcid;
    //! phi id of TC
    int m_phiid;
    //! Theta id of TC
    int m_thetaid;
    //! TC Energy and Timing
    //! Raw TC energy
    double m_raw_energy;
    //! Raw TC timing
    double m_raw_timing;
    //! Raw TC total energy
    double m_raw_energy_tot;
    //! Raw TC total timing
    double m_raw_timing_tot;
    //! Fitting energy
    double m_fit_energy;
    //! Fitting timing
    double m_fit_timing;
    //! beam bkg tag
    int m_beambkgtag;
    //! Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    //! Set TC id
    void setTCId(int tcid) { m_tcid = tcid; }
    //! Set Phi id
    void setPhiId(int phiid) { m_phiid = phiid; }
    //! Set Theta id
    void setThetaId(int thetaid) { m_thetaid = thetaid; }
    //! Set Energy and Timing
    //! Set Raw  enrgy
    void setRawEnergy(double raw_energy) { m_raw_energy = raw_energy; }
    //! Set Raw Timing
    void setRawTiming(double raw_timing) { m_raw_timing = raw_timing; }
    //! Set Raw total Energy
    void setRawEnergyTot(double raw_energy_tot) { m_raw_energy_tot = raw_energy_tot; }
    //! Set Raw Total Timing
    void setRawTimingTot(double raw_timing_tot) { m_raw_timing_tot = raw_timing_tot; }
    //! Set fitting energy
    void setFitEnergy(double fit_energy) { m_fit_energy = fit_energy; }
    //! Set fitting timing
    void setFitTiming(double fit_timing) { m_fit_timing = fit_timing; }
    //! Set Beam background veto
    void setBeamBkgTag(int beambkgtag) { m_beambkgtag = beambkgtag; }


    //! Get event id
    int getEventId() const { return m_eventid; }
    //! Get TC id
    int getTCId() const { return m_tcid; }
    //! Get phi Id
    int getPhiId() const { return m_phiid; }
    //! Get theta Id
    int getThetaId() const { return m_thetaid; }
    //! Get Energy and Timing
    //! Get raw energy
    double getRawEnergy() const {return m_raw_energy; }
    //! Get raw timing
    double getRawTiming() const {return m_raw_timing; }
    //! Get raw total energy
    double getRawEnergyTot() const {return m_raw_energy_tot; }
    //! Get raw total timing
    double getRawTimingTot() const {return m_raw_timing_tot; }
    //! get fit energy
    double getFitEnergy() const {return m_fit_energy; }
    //! Get fit timing
    double getFitTiming() const {return m_fit_timing; }
    //! Get beam background tag
    int getBeamBkgTag() const {return m_beambkgtag; }


    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLFAMAna()
    {
      m_eventid = 0;
      m_tcid = 0;
      m_phiid = 0;
      m_thetaid = 0;
      m_beambkgtag = 0;

      m_raw_energy = 0;
      m_raw_timing = 0;
      m_raw_energy_tot = 0;
      m_raw_timing_tot = 0;
      m_fit_energy = 0;
      m_fit_timing = 0;




    }

    //! Useful Constructor
    TRGECLFAMAna(
      int eventid,
      int tcid,
      int phiid,
      int thetaid,
      int beambkgtag,
      double raw_energy,
      double raw_timing,
      double raw_energy_tot,
      double raw_timing_tot,
      double fit_energy,
      double fit_timing
    )
    {
      m_eventid = eventid;
      m_tcid    = tcid;
      m_phiid   = phiid;
      m_thetaid = thetaid;
      m_raw_energy = raw_energy;
      m_raw_timing = raw_timing;
      m_raw_energy_tot = raw_energy_tot;
      m_raw_timing_tot = raw_timing_tot;
      m_fit_energy = fit_energy;
      m_fit_timing = fit_timing;
      m_beambkgtag = beambkgtag;


    }
    //! the class title
    ClassDef(TRGECLFAMAna, 1); /*< the class title */
  };
} //! End namespace Belle2

#endif
