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

    //! Cluster Theta of this hit.
    int e_cl_theta[6];

    //! Cluster Phi of this hit.
    int e_cl_phi[6];

    //! Cluster time of this hit.
    int e_cl_time[6];

    //! Cluster energy of this hit.
    int e_cl_energy[6];

    //! Cluster CM 1GeV Flag of this hit.
    int e_cl_1gev[6];

    //! Cluster CM 2GeV Flag of this hit.
    int e_cl_2gev[6];

    //! Cluster Bhabha Flag of this hit.
    int e_cl_bha[6];

    //! Number of Cluster of this hit.
    int e_ncl;

    //! Lowmilti bit of this hit.
    int e_low_multi;

    //! 3D Bhabha for veto bit of this hit.
    int e_b2bhabhav;

    //! 3D Bhabha for selection bit of this hit.
    int e_b2bhabhas;

    //! mumu bit of this hit.
    int e_mumu;

    //! prescale bit of this hit.
    int e_prescale;

    //! ICN of this hit.
    int e_icn;

    //! ICN overflow bit of this hit.
    int e_icn_over;

    //! Total Energy Type of this hit.
    int e_etot_type;

    //! Total Energy of this event (Physics region).
    int e_etot;

    //! The case of Total Energy > 20GeV  of this hit.
    int e_ecl_bst;

    //! 2D Bhabha bit of this event.
    int e_b1bhabha;

    //! 2D Bhabha Type of this hit.
    int e_b1_type;

    //! Physics bit of this event.
    int e_physics;

    //! Timing Type of this hit.
    int e_time_type;

    //! checksum flag of this event.
    int e_checksum;

    //! event check of this event.
    int e_checkevt;

    //! trigger type
    int e_trgtype;

    //! Total Energy of this event (All region).
    int e_etot_all;

    //! Event Timing Range min of this event.
    int e_evt_time_min;

    //! Event Timing Range max of this event.
    int e_evt_time_max;

    //! Event Timing win of this event. -1 or 1
    int e_evt_time_win;

    //! The method to set event id
    void setEventId(int eventId) {e_eventId = eventId;}

    //! The method to set the ETM version
    void setETM(int etm) {e_etm = etm;}

    //! The method to set the L1 revoclk
    void setL1Revo(int l1_revo) {e_l1_revo = l1_revo;}

    //! The method to set event Timing
    void setEvtTime(int evt_time) {e_evt_time = evt_time;}

    //! The method to set event revoclk
    void setEvtRevo(int evt_revo) {e_evt_revo = evt_revo;}

    //! The method to set event window
    void setEvtWin(int evt_win) {e_evt_win = evt_win;}

    //! The method to set event ntc
    void setNTC(int ntc) {e_ntc = ntc;}

    //! The method to set Cluster Theta
    void setCLTheta(int cl_theta[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_theta[i] = cl_theta[i];
      }
    }

    //! The mothod to set Cluster Phi
    void setCLPhi(int cl_phi[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_phi[i] = cl_phi[i];
      }
    }

    //! The mothod to set Cluster time
    void setCLTime(int cl_time[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_time[i] = cl_time[i];
      }
    }

    //! The mothod to set Cluster energy
    void setCLEnergy(int cl_energy[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_energy[i] = cl_energy[i];
      }
    }

    //! The mothod to set 1GeV Flag
    void setCLF1GeV(int cl_1gev[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_1gev[i] = cl_1gev[i];
      }
    }

    //! The mothod to set 2GeV Flag
    void setCLF2GeV(int cl_2gev[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_2gev[i] = cl_2gev[i];
      }
    }

    //! The mothod to set Bha Flag
    void setCLFBha(int cl_bha[6])
    {
      for (int i = 0; i < 6; i++) {
        e_cl_bha[i] = cl_bha[i];
      }
    }

    //! The mothod to set Number of Cluster
    void setNCL(int ncl) {e_ncl = ncl;}

    //! The mothod to set Lowmilti bit
    void setLowMulti(int low_multi) {e_low_multi = low_multi;}

    //! The method to set 3D Bhabha veto bit
    void set3DBhabhaV(int b2bhabhav) {e_b2bhabhav = b2bhabhav;}

    //! The method to set 3D Bhabha selection bit
    void set3DBhabhaS(int b2bhabhas) {e_b2bhabhas = b2bhabhas;}

    //! The method to set mumu bit
    void setMumu(int mumu) {e_mumu = mumu;}

    //! The method to set prescale bit
    void setPrescale(int prescale) {e_prescale = prescale;}

    //! The mothod to set ICN
    void setICN(int icn) {e_icn = icn;}

    //! The mothod to set ICN overflow bit
    void setICNOver(int icn_over) {e_icn_over = icn_over;}

    //! The mothod to set Total Energy Type
    void setEtotType(int etot_type) {e_etot_type = etot_type;}

    //! The method to set Total Energy
    void setEtot(int etot) {e_etot = etot;}

    //! The mothod to set Total Energy more than 20 gev
    void setECLBST(int eclbst) {e_ecl_bst = eclbst;}

    //! The method to set 2D Bhabha bit
    void set2DBhabha(int b1bhabha) {e_b1bhabha = b1bhabha;}

    //! The mothod to set 2D Bhabha Type
    void setBhabhaType(int b1_type) {e_b1_type = b1_type;}

    //! The method to set Physics bit
    void setPhysics(int physics) {e_physics = physics;}

    //! The mothod to set Timing Type
    void setTimeType(int time_type) {e_time_type = time_type;}

    //! The method to set checksum flag
    void setCheckSum(int checksum) {e_checksum = checksum;}

    //! The method to set Evt Exist
    void setEvtExist(int evtexist) {e_checkevt = evtexist;}

    //! The method to set TRG Type
    void setTRGTYPE(int trgtype) {e_trgtype = trgtype;}

    //! The method to set ETOT all
    void setEtotAll(int etotall) {e_etot_all = etotall;}

    //! The method to set Evt Timing Min
    void setEvtTimeMin(int timemin) {e_evt_time_min = timemin;}

    //! The method to set Evt Timing Max
    void setEvtTimeMax(int timemax) {e_evt_time_max = timemax;}

    //! The method to set Evt Timing Win
    void setEvtTimeWin(int timewin) {e_evt_time_win = timewin;}

    //! The method to get event id
    int getEventId() const {return e_eventId;}

    //! The method to get the ETM version
    int getETM() const {return e_etm;}

    //! The method to get the L1 revoclk
    int getL1Revo() const {return e_l1_revo;}

    //! The method to get event Timing
    int getEvtTime() const {return e_evt_time;}

    //! The method to get event revoclk
    int getEvtRevo() const {return e_evt_revo;}

    //! The method to get event window
    int getEvtWin() const {return e_evt_win;}

    //! The method to get event ntc
    int getNTC() const {return e_ntc;}

    //! The method to get Cluster Theta
    const int* getCLTheta() const {return e_cl_theta;}

    //! The mothod to get Cluster Phi
    const int* getCLPhi() const {return e_cl_phi;}

    //! The mothod to get Cluster time
    const int* getCLTime() const {return e_cl_time;}

    //! The mothod to get Cluster energy
    const int* getCLEnergy() const {return e_cl_energy;}

    //! The mothod to get 1GeV Flag
    const int* getCL1GeV() const {return e_cl_1gev;}

    //! The mothod to get 2GeV Flag
    const int* getCL2GeV() const {return e_cl_2gev;}

    //! The mothod to get Bhabha Flag
    const int* getCLBha() const {return e_cl_bha;}

    //! The mothod to get Number of Cluster
    int getNCL() const {return e_ncl;}

    //! The mothod to get Lowmilti bit
    int getLowMulti() const {return e_low_multi;}

    //! The method to get 3D Bhabha veto bit
    int get3DBhabhaV() const {return e_b2bhabhav;}

    //! The method to get 3D Bhabha selection bit
    int get3DBhabhaS() const {return e_b2bhabhas;}

    //! The method to get mumu bit
    int getMumu() const {return e_mumu;}

    //! The method to get prescale bit
    int getPrescale() const {return e_prescale;}

    //! The mothod to get ICN
    int getICN() const {return e_icn;}

    //! The mothod to get ICN overflow bit
    int getICNOver() const {return e_icn_over;}

    //! The mothod to get Total Energy Type
    int getEtotType() const {return e_etot_type;}

    //! The method to get Total Energy
    int getEtot() const {return e_etot;}

    //! The mothod to get Total Energy more than 20GeV
    int getECLBST() const {return e_ecl_bst;}

    //! The method to get 2D Bhabha bit
    int get2DBhabha() const {return e_b1bhabha;}

    //! The mothod to get 2D Bhabha Type
    int getBhabhaType() const {return e_b1_type;}

    //! The method to get Physics bit
    int getPhysics() const {return e_physics;}

    //! The method to get checksum flag
    int getCheckSum() const {return e_checksum;}

    //! The method to get Evt Check
    int getEvtExist() const {return e_checkevt;}

    //! The method to get TRG Type
    int getTRGTYPE() const {return e_trgtype;}

    //! The mothod to get Timing Type
    int getTimeType() const {return e_time_type;}

    //! The mothod to get ETOT all
    int getEtotAll() const {return e_etot_all;}

    //! The mothod to get Evt Timing Min
    int getEvtTimeMin() const {return e_evt_time_min;}

    //! The mothod to get Evt Timing Max
    int getEvtTimeMax() const {return e_evt_time_max;}

    //! The mothod to get Evt Timing Win
    int getEvtTimeWin() const {return e_evt_time_win;}

    TRGECLUnpackerEvtStore()
    {
      e_eventId      = 0;
      e_etm          = 0;
      e_l1_revo      = 0;
      e_evt_time     = 0;
      e_evt_revo     = 0;
      e_evt_win      = 0;
      e_ntc          = 0;
      memset(e_cl_theta,   0, sizeof(e_cl_theta));
      memset(e_cl_phi,     0, sizeof(e_cl_phi));
      memset(e_cl_time,    0, sizeof(e_cl_time));
      memset(e_cl_energy,  0, sizeof(e_cl_energy));
      memset(e_cl_1gev,    0, sizeof(e_cl_1gev));
      memset(e_cl_2gev,    0, sizeof(e_cl_2gev));
      memset(e_cl_bha,     0, sizeof(e_cl_bha));
      e_ncl          = 0;
      e_low_multi    = 0;
      e_b2bhabhav    = 0;
      e_b2bhabhas    = 0;
      e_mumu         = 0;
      e_prescale     = 0;
      e_icn          = 0;
      e_icn_over     = 0;
      e_etot_type    = 0;
      e_etot         = 0;
      e_ecl_bst      = 0;
      e_b1bhabha     = 0;
      e_b1_type      = 0;
      e_physics      = 0;
      e_time_type    = 0;
      e_checksum     = 0;
      e_checkevt     = 0;
      e_trgtype      = 0;
      e_etot_all     = 0;
      e_evt_time_min = 0;
      e_evt_time_max = 0;
      e_evt_time_win = 0;
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
      int cl_theta[6],
      int cl_phi[6],
      int cl_time[6],
      int cl_energy[6],
      int cl_1gev[6],
      int cl_2gev[6],
      int cl_bha[6],
      int ncl,
      int low_multi,
      int b2bhabhav,
      int b2bhabhas,
      int mumu,
      int prescale,
      int icn,
      int icn_over,
      int etot_type,
      int etot,
      int eclbst,
      int b1bhabha,
      int b1_type,
      int physics,
      int time_type,
      int checksum,
      int evtexist,
      int trgtype,
      int etotall,
      int timemin,
      int timemax,
      int timewin
    )
    {
      e_eventId      = eventId;
      e_etm          = etm;
      e_l1_revo      = l1_revo;
      e_evt_time     = evt_time;
      e_evt_revo     = evt_revo;
      e_evt_win      = evt_win;
      e_ntc          = ntc;
      for (int i = 0; i < 6; i++) {
        e_cl_theta[i]   = cl_theta[i];
        e_cl_phi[i]     = cl_phi[i];
        e_cl_time[i]    = cl_time[i];
        e_cl_energy[i]  = cl_energy[i];
        e_cl_1gev[i]    = cl_1gev[i];
        e_cl_2gev[i]    = cl_2gev[i];
        e_cl_bha[i]     = cl_bha[i];
      }
      e_ncl          = ncl;
      e_low_multi    = low_multi;
      e_b2bhabhav    = b2bhabhav;
      e_b2bhabhas    = b2bhabhas;
      e_mumu         = mumu;
      e_prescale     = prescale;
      e_icn          = icn;
      e_icn_over     = icn_over;
      e_etot_type    = etot_type;
      e_etot         = etot;
      e_ecl_bst      = eclbst;
      e_b1bhabha     = b1bhabha;
      e_b1_type      = b1_type;
      e_physics      = physics;
      e_time_type    = time_type;
      e_checksum     = checksum;
      e_checkevt     = evtexist;
      e_trgtype      = trgtype;
      e_etot_all     = etotall;
      e_evt_time_min = timemin;
      e_evt_time_max = timemax;
      e_evt_time_win = timewin;
    }
    /** the class title */
    ClassDef(TRGECLUnpackerEvtStore, 4);
  };

} // end namespace Belle2

#endif
