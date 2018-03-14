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

    //! The EVT Timing of TRG Timing
    int m_evttime;

    //! The revo clk of FAM
    int m_revo_fam;

    //! The revo clk of gdl
    int m_revo_gdl;

    //! The window of hit
    int m_hit_win;

    //! The checksum flag
    int m_check_sum;

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

    //! The method to set evt timing
    void setEVTTime(int EVTTIME) { m_evttime = EVTTIME;}

    //! The method to set revo clock of FAM
    void setRevoFAM(int RevoFAM) { m_revo_fam = RevoFAM;}

    //! The method to set revo clock of GDL
    void setRevoGDL(int RevoGDL) { m_revo_gdl = RevoGDL;}

    //! The method to set checksum flag
    void setChecksum(int Checksum) { m_check_sum = Checksum;}

    //! The method to set Hit window
    void setHitWin(int HitWin) { m_hit_win = HitWin;}

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

    //! The method to get evt timing
    int getEVTTime() const {return m_evttime;}

    //! The method to get revo clock of FAM
    int getRevoFAM() const {return m_revo_fam;}

    //! The method to get revo clock of GDL
    int getRevoGDL() const {return m_revo_gdl;}

    //! The method to get checksum flag
    int getChecksum() const {return m_check_sum;}

    //! The method to get Hit Win
    int getHitWin() const {return m_hit_win;}


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
      m_evttime    = 0;
      m_revo_fam   = 0;
      m_revo_gdl   = 0;
      m_check_sum  = 0;
      m_hit_win    = 0;
    }

    //! Useful Constructor
    TRGECLUnpackerStore(
      int eventId,
      int TCId,
      int NTC,
      int TCE,
      int TCT,
      int TCCALT,
      int EVTTIME,
      int RevoFAM,
      int RevoGDL,
      int Checksum,
      int HitWin
    )
    {
      m_eventId    = eventId;
      m_hittedTCId = TCId;
      m_ntc        = NTC;
      m_energy     = TCE;
      m_timing     = TCT;
      m_caltime    = TCCALT;
      m_evttime    = EVTTIME;
      m_revo_fam   = RevoFAM;
      m_revo_gdl   = RevoGDL;
      m_check_sum  = Checksum;
      m_hit_win    = HitWin;
    }
    /** the class title */
    ClassDef(TRGECLUnpackerStore, 1);

  };

} // end namespace Belle2

#endif
