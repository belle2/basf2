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

#ifndef TRGECLTCHIT_H
#define TRGECLTCHIT_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLDigi : public TObject {
  public:

    // Event Id
    int m_eventid;
    // TC id
    int m_tcid;
    int m_phiid;
    int m_thetaid;
    int m_tcnoutput;
    // TC Energy and Timing
    double m_raw_energy;
    double m_raw_timing;
    double m_raw_energy_tot;
    double m_raw_timing_tot;
    double m_fit_energy;
    double m_fit_timing;

    // Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    // Set TC id
    void setTCId(int tcid) { m_tcid = tcid; }
    void setPhiId(int phiid) { m_phiid = phiid; }
    void setThetaId(int thetaid) { m_thetaid = thetaid; }
    // Set Energy and Timing
    void setRawEnergy(double raw_energy) { m_raw_energy = raw_energy; }
    void setRawTiming(double raw_timing) { m_raw_timing = raw_timing; }
    void setRawEnergyTot(double raw_energy_tot) { m_raw_energy_tot = raw_energy_tot; }
    void setRawTimingTot(double raw_timing_tot) { m_raw_timing_tot = raw_timing_tot; }
    void setFitEnergy(double fit_energy) { m_fit_energy = fit_energy; }
    void setFitTiming(double fit_timing) { m_fit_timing = fit_timing; }

    void setTCnOutput(int TC_noutput) { m_tcnoutput = TC_noutput; }

    // Get event id
    int getEventId() const { return m_eventid; }
    // Get TC id
    int getTCId() const { return m_tcid; }
    int getPhiId() const { return m_phiid; }
    int getThetaId() const { return m_thetaid; }
    // Get Energy and Timing
    double getRawEnergy() const {return m_raw_energy; }
    double getRawTiming() const {return m_raw_timing; }
    double getRawEnergyTot() const {return m_raw_energy_tot; }
    double getRawTimingTot() const {return m_raw_timing_tot; }
    double getFitEnergy() const {return m_fit_energy; }
    double getFitTiming() const {return m_fit_timing; }

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLDigi() {;}

    // Useful Constructor
    TRGECLDigi(
      int eventid,
      int tcid,
      int phiid,
      int thetaid,
      double raw_energy,
      double raw_timing,
      double raw_energy_tot,
      double raw_timing_tot,
      double fit_energy,
      double fit_timing
    ) {
      m_eventid = eventid;
      m_tcid    = tcid;
      m_phiid   = phiid;
      m_thetaid = thetaid;
      m_raw_energy = raw_energy;
      m_raw_timing = raw_timing;
      m_raw_energy = raw_energy_tot;
      m_raw_timing = raw_timing_tot;
      m_fit_energy = fit_energy;
      m_fit_timing = fit_timing;
    }
    ClassDef(TRGECLDigi, 1); /*< the class title */
  };
} // end namespace Belle2

#endif
