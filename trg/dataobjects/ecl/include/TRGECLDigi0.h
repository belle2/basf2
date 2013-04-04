//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLDigi0.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//-----------------------------------------------------------
// Description : A class to represent ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------

#ifndef TRGECLDIGI0_H
#define TRGECLDIGI0_H

#include <TObject.h>
#include <TVector3.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLDigi0 : public TObject {
  public:

    // Event Id
    int m_eventid;
    // TC id
    int m_tcid;
    //
    int m_ibintime;
    // TC Energy and Timing
    double m_raw_energy;
    double m_raw_timing;
    // Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    // Set TC id
    void setTCId(int tcid) { m_tcid = tcid; }
    //
    void setiBinTime(int ibintime) { m_ibintime = ibintime; }
    // Set Energy and Timing
    void setRawEnergy(double raw_energy) { m_raw_energy = raw_energy; }
    void setRawTiming(double raw_timing) { m_raw_timing = raw_timing; }
    // Get event id
    int getEventId() const { return m_eventid; }
    // Get TC id
    int getTCId() const { return m_tcid; }
    //
    int getiBinTime() const { return m_ibintime; }
    // Get Energy and Timing
    double getRawEnergy() const {return m_raw_energy; }
    double getRawTiming() const {return m_raw_timing; }

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLDigi0() {;}

    // Useful Constructor
    TRGECLDigi0(
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
    ClassDef(TRGECLDigi0, 1); /*< the class title */
  };
} // end namespace Belle2

#endif
