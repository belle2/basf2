//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLUnpackerStore.h
// Section  : TRG ECL
// Owner    : SungHyun Kim
// Email    : sunghyun.kim@belle2.org
//-----------------------------------------------------------
// Description : A class to represent TRG ECL.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGECLUNPACKERSTORE_H
#define TRGECLUNPACKERSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLUnpackerStore : public TObject {
  public:


    //! The evt id of this hit.
    int m_eventId;

    //! The Number of Hitted TC from summary
    int m_ntc;

    //! The Hitted TC id
    int m_hittedTCId;

    //! The Energy of hitted TC
    int m_energy;

    //! The Total Timing of hitted TC
    int m_timing;

    //! The Timing of TRG Timing
    int m_caltime;

    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId;}

    //! The method to set ntc
    void setNTC(int NTC) { m_ntc = NTC; }

    //! The method to set cell id
    void setTCId(int TCId) { m_hittedTCId = TCId;}

    //! The method to set deposited energy
    void setTCEnergy(int TCE) { m_energy = TCE;}

    //! The method to set hit time
    void setTCTime(int TCT) { m_timing = TCT;}

    //! The method to set cal timing
    void setTCCALTime(int TCCALT) { m_caltime = TCCALT;}

    //! The method to get event id
    int getEventId() const { return m_eventId;}

    //! The method to get ntc
    int getNTC() const { return m_ntc; }

    //! The method to get cell id
    int getTCId() const { return m_hittedTCId;}

    //! The method to get deposited energy
    int getTCEnergy() const { return m_energy;}

    //! The method to get hit average time
    int getTCTime() const {return m_timing;}

    //! The method to get cal timing
    int getTCCALTime() const {return m_caltime;}

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLUnpackerStore()
    {
      m_eventId    = 0;
      m_hittedTCId = 0;
      m_ntc        = 0;
      m_energy     = 0;
      m_timing     = 0;
      m_caltime    = 0;
    }

    //! Useful Constructor
    TRGECLUnpackerStore(
      int eventId,
      int TCId,
      int NTC,
      int TCE,
      int TCT,
      int TCCALT
    )
    {
      m_eventId    = eventId;
      m_hittedTCId = TCId;
      m_ntc        = NTC;
      m_energy     = TCE;
      m_timing     = TCT;
      m_caltime    = TCCALT;
    }
    /** the class title */
    ClassDef(TRGECLUnpackerStore, 1);

  };

} // end namespace Belle2

#endif
