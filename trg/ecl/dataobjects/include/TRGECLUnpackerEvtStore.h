//-----------------------------------------------------------
// $Id$
//-----------------------------------------------------------
// Filename : TRGECLUnpackerEvtStore.h
// Section  : TRG ECL
// Owner    : SungHyun Kim
// Email    : sunghyun.kim@belle2.org
//-----------------------------------------------------------
// Description : A class to represent TRG ECL Event.
//-----------------------------------------------------------
// $Log$
//-----------------------------------------------------------
#ifndef TRGECLUNPACKEREVTSTORE_H
#define TRGECLUNPACKEREVTSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLUnpackerEvtStore : public TObject {
  public:

    //! The evt id of this event.
    int e_eventId;

    //! The ETM version of this event.
    int e_etm;

    //! The L1 revoclk of this event.
    int e_l1_revo;

    //! Event Timing of this event.
    int e_evt_time;

    //! Event revoclk of this event.
    int e_evt_revo;

    //! Event window of this event.
    int e_evt_win;

    //! Number of TC of this event.
    int e_ntc;

    //! Total Energy of this event.
    int e_etot;

    //! Total Energy in physics region of this event.
    int e_etot_p;

    //! 3D Bhabha bit of this event.
    int e_b2bhabha;

    //! 2D Bhabha bit of this event.
    int e_b1bhabha;

    //! Physics bit of this event.
    int e_physics;

    //! checksum flag of this event.
    int e_checksum;

    //! The method to set event id
    void setEventId(int eventId) {e_eventId = eventId;}

    //! The mothod to set the ETM version
    void setETM(int etm) {e_etm = etm;}

    //! The mothod to set the L1 revoclk
    void setL1Revo(int l1_revo) {e_l1_revo = l1_revo;}

    //! The mothod to set event Timing
    void setEvtTime(int evt_time) {e_evt_time = evt_time;}

    //! The mothod to set event revoclk
    void setEvtRevo(int evt_revo) {e_evt_revo = evt_revo;}

    //! The mothod to set event window
    void setEvtWin(int evt_win) {e_evt_win = evt_win;}

    //! The mothod to set event ntc
    void setNTC(int ntc) {e_ntc = ntc;}

    //! The mothod to set Total Energy
    void setEtot(int etot) {e_etot = etot;}

    //! The mothod to set Total Energy in physics region
    void setEtotP(int etot_p) {e_etot_p = etot_p;}

    //! The mothod to set 3D Bhabha bit
    void set3DBhabha(int b2bhabha) {e_b2bhabha = b2bhabha;}

    //! The mothod to set 2D Bhabha bit
    void set2DBhabha(int b1bhabha) {e_b1bhabha = b1bhabha;}

    //! The mothod to set Physics bit
    void setPhysics(int physics) {e_physics = physics;}

    //! The mothod to set checksum flag
    void setCheckSum(int checksum) {e_checksum = checksum;}

    //! The method to get event id
    int getEventId() const {return e_eventId;}

    //! The mothod to get the ETM version
    int setETM() const {return e_etm;}

    //! The mothod to get the L1 revoclk
    int getL1Revo() const {return e_l1_revo;}

    //! The mothod to get event Timing
    int getEvtTime() const {return e_evt_time;}

    //! The mothod to get event revoclk
    int getEvtRevo() const {return e_evt_revo;}

    //! The mothod to get event window
    int getEvtWin() const {return e_evt_win;}

    //! The mothod to get event ntc
    int getNTC() const {return e_ntc;}

    //! The mothod to get Total Energy
    int getEtot() const {return e_etot;}

    //! The mothod to get Total Energy in physics region
    int getEtotP() const {return e_etot_p;}

    //! The mothod to get 3D Bhabha bit
    int get3DBhabha() const {return e_b2bhabha;}

    //! The mothod to get 2D Bhabha bit
    int get2DBhabha() const {return e_b1bhabha;}

    //! The mothod to get Physics bit
    int getPhysics() const {return e_physics;}

    //! The mothod to get checksum flag
    int getCheckSum() const {return e_checksum;}

    TRGECLUnpackerEvtStore()
    {

      e_eventId   = 0;
      e_etm       = 0;
      e_l1_revo   = 0;
      e_evt_time  = 0;
      e_evt_revo  = 0;
      e_evt_win   = 0;
      e_ntc       = 0;
      e_etot      = 0;
      e_etot_p    = 0;
      e_b2bhabha  = 0;
      e_b1bhabha  = 0;
      e_physics   = 0;
      e_checksum  = 0;
    }

    //! Useful Constructor
    TRGECLUnpackerEvtStore(
      int eventId,
      int etm,
      int l1_revo,
      int evt_time,
      int evt_revo,
      int evt_win,
      int ntc,
      int etot,
      int etot_p,
      int b2bhabha,
      int b1bhabha,
      int physics,
      int checksum
    )
    {
      e_eventId  = eventId;
      e_etm      = etm;
      e_l1_revo  = l1_revo;
      e_evt_time = evt_time;
      e_evt_revo = evt_revo;
      e_evt_win  = evt_win;
      e_ntc      = ntc;
      e_etot     = etot;
      e_etot_p   = etot_p;
      e_b2bhabha = b2bhabha;
      e_b1bhabha = b1bhabha;
      e_physics  = physics;
      e_checksum = checksum;
    }
    /** the class title */
    ClassDef(TRGECLUnpackerEvtStore, 1);
  };

} // end namespace Belle2

#endif
