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

    //! The Hitted TC id
    int m_tcid;

    //! The Total Timing of hitted TC
    int m_time;

    //! The Timing of TRG Timing
    int m_caltime;

    //! The window of hit
    int m_hit_win;

    //! The revo clk of FAM
    int m_revo_fam;

    //! The EVT Timing of TRG Timing
    int m_evttime;

    //! The revo clk of gdl
    int m_revo_gdl;

    //! The Energy of hitted TC
    int m_energy;

    //! The checksum flag
    int m_checksum;

    //! The method to set event id
    void setEventId(int eventId) { m_eventId = eventId;}

    //! The method to set cell id
    void setTCId(int TCId) { m_tcid = TCId;}

    //! The method to set hit time
    void setTCTime(int TCT) { m_time = TCT;}

    //! The method to set evt timing
    void setEVTTime(int EVTTIME) { m_evttime = EVTTIME;}

    //! The method to set cal timing
    void setTCCALTime(int TCCALT) { m_caltime = TCCALT;}

    //! The method to set Hit window
    void setHitWin(int HitWin) { m_hit_win = HitWin;}

    //! The method to set revo clock of FAM
    void setRevoFAM(int RevoFAM) { m_revo_fam = RevoFAM;}

    //! The method to set revo clock of GDL
    void setRevoGDL(int RevoGDL) { m_revo_gdl = RevoGDL;}

    //! The method to set deposited energy
    void setTCEnergy(int TCE) { m_energy = TCE;}

    //! The method to set checksum flag
    void setChecksum(int checksum) { m_checksum = checksum;}

    //! The method to get event id
    int getEventId() const { return m_eventId;}

    //! The method to get cell id
    int getTCId() const { return m_tcid;}

    //! The method to get hit average time
    int getTCTime() const {return m_time;}

    //! The method to get evt timing
    int getEVTTime() const {return m_evttime;}

    //! The method to get cal timing
    int getTCCALTime() const {return m_caltime;}

    //! The method to get revo clock of FAM
    int getRevoFAM() const {return m_revo_fam;}

    //! The method to get revo clock of GDL
    int getRevoGDL() const {return m_revo_gdl;}

    //! The method to get Hit Win
    int getHitWin() const {return m_hit_win;}

    //! The method to get deposited energy
    int getTCEnergy() const { return m_energy;}

    //! The method to get checksum flag
    int getChecksum() const {return m_checksum;}

    // Empty constructor
    // Recommended for ROOT IO
    TRGECLUnpackerStore()
    {
      m_eventId    = 0;
      m_tcid       = 0;
      m_time       = 0;
      m_caltime    = 0;
      m_hit_win    = 0;
      m_revo_fam   = 0;
      m_energy     = 0;
      m_checksum   = 0;
      m_evttime    = 0;
      m_revo_gdl   = 0;


    }

    //! Useful Constructor
    TRGECLUnpackerStore(
      int eventId,
      int TCId,
      int TCT,
      int TCCALT,
      int EVTTIME,
      int HitWin,
      int RevoFAM,
      int RevoGDL,
      int TCE,
      int checksum
    )
    {
      m_eventId    = eventId;
      m_tcid       = TCId;
      m_time       = TCT;
      m_caltime    = TCCALT;
      m_evttime    = EVTTIME;
      m_hit_win    = HitWin;
      m_revo_fam   = RevoFAM;
      m_revo_gdl   = RevoGDL;
      m_energy     = TCE;
      m_checksum   = checksum;
    }
    /** the class title */
    ClassDef(TRGECLUnpackerStore, 2);

  };

} // end namespace Belle2

#endif
