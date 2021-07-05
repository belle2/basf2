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

namespace Belle2 {

  //! Raw TC result nefor digitizing
  class TRGECLDigi0 : public TObject {
  public:

    //! Event Id
    int m_eventid;
    //! TC id
    int m_tcid;
    //! time bin
    int m_ibintime;
    //! beam background tag
    int m_beambkgtag;
    //! TC Energy and Timing
    //!  Raw TC energy
    double m_raw_energy;
    //! Raw TC timing
    double m_raw_timing;


    //! Set event id
    void setEventId(int eventid) { m_eventid = eventid; }
    //! Set TC id
    void setTCId(int tcid) { m_tcid = tcid; }
    //! Set time bin
    void setiBinTime(int ibintime) { m_ibintime = ibintime; }
    //! Set Energy and Timing
    //! set raw energy
    void setRawEnergy(double raw_energy) { m_raw_energy = raw_energy; }
    //! Set raw timing
    void setRawTiming(double raw_timing) { m_raw_timing = raw_timing; }
    //! Set beam background tag
    void setBeamBkgTag(int beambkgtag) { m_beambkgtag = beambkgtag; }


    //! Get event id
    int getEventId() const { return m_eventid; }
    //! Get TC id
    int getTCId() const { return m_tcid; }
    //! get time bin
    int getiBinTime() const { return m_ibintime; }
    //! Get Energy and Timing
    //! Get raw TC energy
    double getRawEnergy() const {return m_raw_energy; }
    //! Get raw TC timing
    double getRawTiming() const {return m_raw_timing; }
    //!  Get beam background tag
    int getBeamBkgTag() const {return m_beambkgtag; }

    //! Empty constructor
    //! Recommended for ROOT IO
    TRGECLDigi0()
    {
      m_eventid    = 0;
      m_tcid       = 0;
      m_ibintime   = 0;
      m_raw_energy = 0;
      m_raw_timing = 0;
      m_beambkgtag = 0;


    }

    //! Useful Constructor
    TRGECLDigi0(
      int eventid,
      int tcid,
      int ibintime,
      int beambkgtag,
      double raw_energy,
      double raw_timing
    )
    {
      m_eventid = eventid;
      m_tcid    = tcid;
      m_ibintime = ibintime;
      m_raw_energy = raw_energy;
      m_raw_timing = raw_timing;
      m_beambkgtag = beambkgtag;

    }
    //! the class title
    ClassDef(TRGECLDigi0, 1); /*< the class title */
  };
} //! end namespace Belle2

#endif
