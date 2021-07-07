/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGECLUNPACKERSUMSTORE_H
#define TRGECLUNPACKERSUMSTORE_H

#include <TObject.h>

namespace Belle2 {

  //! Example Detector
  class TRGECLUnpackerSumStore : public TObject {
  public:

    //! The evt id of this hit.
    int s_eventId;

    //! The summary window of this hit.
    int s_sum_win = 0;

    //! The summary revoclk of this hit.
    int s_sum_revo = 0;

    //! Cluster Theta of this hit.
    int s_cl_theta[6];

    //! Cluster Phi of this hit.
    int s_cl_phi[6];

    //! Cluster time of this hit.
    int s_cl_time[6];

    //! Cluster energy of this hit.
    int s_cl_energy[6];

    //! Cluster CM 1GeV Flag of this hit.
    int s_cl_1gev[6];

    //! Cluster CM 2GeV Flag of this hit.
    int s_cl_2gev[6];

    //! Cluster Bhabha Flag of this hit.
    int s_cl_bha[6];

    //! Number of Cluster of this hit.
    int s_ncl;

    //! ICN of this hit.
    int s_icn;

    //! ICN overflow bit of this hit.
    int s_icn_over;

    //! Lowmilti bit of this hit.
    int s_low_multi;

    //! 3D Bhabha for veto bit of this hit.
    int s_b2bhabhav;

    //! 3D Bhabha for selection bit of this hit.
    int s_b2bhabhas;

    //! mumu bit of this hit.
    int s_mumu;

    //! prescale bit of this hit.
    int s_prescale;

    //! 2D Bhabha bit of this hit.
    int s_b1bhabha;

    //! 2D Bhabha Type of this hit.
    int s_b1_type;

    //! Physics bit of this hit.
    int s_physics;

    //! BG Veto bit of this hit.
    int s_bg_veto;

    //! Total Energy of this hit.
    int s_etot;

    //! Total Energy Type of this hit.
    int s_etot_type;

    //! The case of ECL Burst  of this hit.
    int s_ecl_bst;

    //! Timing of this hit.
    int s_time;

    //! Timing Type of this hit.
    int s_time_type;

    //! The method to set event id
    void setEventId(int eventId) {s_eventId = eventId;}

    //! The mothod to set the sumamry window
    void setSumNum(int sum_win) {s_sum_win = sum_win;}

    //! The mothod to set the sumamry revoclk
    void setSumRevo(int sum_revo) {s_sum_revo = sum_revo;}

    //! The mothod to set Timing
    void setTime(int time) {s_time = time;}

    //! The method to set Cluster Theta
    void setCLTheta(int cl_theta[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_theta[i] = cl_theta[i];
      }
    }

    //! The mothod to set Cluster Phi
    void setCLPhi(int cl_phi[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_phi[i] = cl_phi[i];
      }
    }

    //! The mothod to set Cluster time
    void setCLTime(int cl_time[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_time[i] = cl_time[i];
      }
    }

    //! The mothod to set Cluster energy
    void setCLEnergy(int cl_energy[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_energy[i] = cl_energy[i];
      }
    }

    //! The mothod to set 1GeV Flag
    void setCLF1GeV(int cl_1gev[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_1gev[i] = cl_1gev[i];
      }
    }

    //! The mothod to set 2GeV Flag
    void setCLF2GeV(int cl_2gev[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_2gev[i] = cl_2gev[i];
      }
    }

    //! The mothod to set Bha Flag
    void setCLFBha(int cl_bha[6])
    {
      for (int i = 0; i < 6; i++) {
        s_cl_bha[i] = cl_bha[i];
      }
    }

    //! The mothod to set Number of Cluster
    void setNCL(int ncl) {s_ncl = ncl;}

    //! The mothod to set ICN
    void setICN(int icn) {s_icn = icn;}

    //! The mothod to set ICN overflow bit
    void setICNOver(int icn_over) {s_icn_over = icn_over;}

    //! The mothod to set Lowmilti bit
    void setLowMulti(int low_multi) {s_low_multi = low_multi;}

    //! The mothod to set 3D Bhabha veto bit
    void set3DBhabhaV(int b2bhabhav) {s_b2bhabhav = b2bhabhav;}

    //! The mothod to set 3D Bhabha selection bit
    void set3DBhabhaS(int b2bhabhas) {s_b2bhabhas = b2bhabhas;}

    //! The mothod to set mumu bit
    void setMumu(int mumu) {s_mumu = mumu;}

    //! The mothod to set prescale bit
    void setPrescale(int prescale) {s_prescale = prescale;}

    //! The mothod to set 2D Bhabha bit
    void set2DBhabha(int b1bhabha) {s_b1bhabha = b1bhabha;}

    //! The mothod to set 2D Bhabha Type
    void setBhabhaType(int b1_type) {s_b1_type = b1_type;}

    //! The mothod to set Physics bit
    void setPhysics(int physics) {s_physics = physics;}

    //! The mothod to set BG Veto bit
    void setBG(int bg_veto) {s_bg_veto = bg_veto;}

    //! The mothod to set Total Energy
    void setEtot(int etot) {s_etot = etot;}

    //! The mothod to set Total Energy Type
    void setEtotType(int etot_type) {s_etot_type = etot_type;}

    //! The mothod to set ECL Burst
    void setECLBST(int ecl_bst) {s_ecl_bst = ecl_bst;}

    //! The mothod to set Timing Type
    void setTimeType(int time_type) {s_time_type = time_type;}

    //! The method to get event id
    int getEventId() const {return s_eventId;}

    //! The mothod to get the sumamry window
    int getSumNum() const {return s_sum_win;}

    //! The mothod to get the sumamry revoclk
    int getSumRevo() const {return s_sum_revo;}

    //! The method to get Cluster Theta
    const int* getCLTheta() const {return s_cl_theta;}

    //! The mothod to get Cluster Phi
    const int* getCLPhi() const {return s_cl_phi;}

    //! The mothod to get Cluster time
    const int* getCLTime() const {return s_cl_time;}

    //! The mothod to get Cluster energy
    const int* getCLEnergy() const {return s_cl_energy;}

    //! The mothod to get 1GeV Flag
    const int* getCL1GeV() const {return s_cl_1gev;}

    //! The mothod to get 2GeV Flag
    const int* getCL2GeV() const {return s_cl_2gev;}

    //! The mothod to get Bhabha Flag
    const int* getCLBha() const {return s_cl_bha;}

    //! The mothod to get Number of Cluster
    int getNCL() const {return s_ncl;}

    //! The mothod to get ICN
    int getICN() const {return s_icn;}

    //! The mothod to get ICN overflow bit
    int getICNOver() const {return s_icn_over;}

    //! The mothod to get Lowmilti bit
    int getLowMulti() const {return s_low_multi;}

    //! The mothod to get 3D Bhabha veto bit
    int get3DBhabhaV() const {return s_b2bhabhav;}

    //! The mothod to get 3D Bhabha selection bit
    int get3DBhabhaS() const {return s_b2bhabhas;}

    //! The mothod to get mumu bit
    int getMumu() const {return s_mumu;}

    //! The mothod to get prescale bit
    int getPrescale() const {return s_prescale;}

    //! The mothod to get 2D Bhabha bit
    int get2DBhabha() const {return s_b1bhabha;}

    //! The mothod to get 2D Bhabha Type
    int getBhabhaType() const {return s_b1_type;}

    //! The mothod to get Physics bit
    int getPhysics() const {return s_physics;}

    //! The mothod to get BG Veto bit
    int getBG() const {return s_bg_veto;}

    //! The mothod to get Total Energy
    int getEtot() const {return s_etot;}

    //! The mothod to get Total Energy Type
    int getEtotType() const {return s_etot_type;}

    //! The mothod to get ECL Burst
    int getECLBST() const {return s_ecl_bst;}

    //! The mothod to get Timing
    int getTime() const {return s_time;}

    //! The mothod to get Timing Type
    int getTimeType() const {return s_time_type;}

    TRGECLUnpackerSumStore()
    {
      s_eventId   = 0;
      memset(s_cl_theta,   0, sizeof(s_cl_theta));
      memset(s_cl_phi,     0, sizeof(s_cl_phi));
      memset(s_cl_time,    0, sizeof(s_cl_time));
      memset(s_cl_energy,  0, sizeof(s_cl_energy));
      memset(s_cl_1gev,    0, sizeof(s_cl_1gev));
      memset(s_cl_2gev,    0, sizeof(s_cl_2gev));
      memset(s_cl_bha,     0, sizeof(s_cl_bha));
      s_ncl        = 0;
      s_icn        = 0;
      s_icn_over   = 0;
      s_low_multi  = 0;
      s_b2bhabhav  = 0;
      s_b2bhabhas  = 0;
      s_mumu       = 0;
      s_prescale   = 0;
      s_b1bhabha   = 0;
      s_b1_type    = 0;
      s_physics    = 0;
      s_bg_veto    = 0;
      s_etot       = 0;
      s_etot_type  = 0;
      s_ecl_bst    = 0;
      s_time       = 0;
      s_time_type  = 0;
    }

    //! Useful Constructor
    TRGECLUnpackerSumStore(
      int eventId,
      int cl_theta[6],
      int cl_phi[6],
      int cl_time[6],
      int cl_energy[6],
      int cl_1gev[6],
      int cl_2gev[6],
      int cl_bha[6],
      int ncl,
      int icn,
      int icn_over,
      int low_multi,
      int b2bhabhav,
      int b2bhabhas,
      int mumu,
      int prescale,
      int b1bhabha,
      int b1_type,
      int physics,
      int bg_veto,
      int etot,
      int etot_type,
      int ecl_bst,
      int time,
      int time_type
    )
    {
      s_eventId   = eventId;
      for (int i = 0; i < 6; i++) {
        s_cl_theta[i]   = cl_theta[i];
        s_cl_phi[i]     = cl_phi[i];
        s_cl_time[i]    = cl_time[i];
        s_cl_energy[i]  = cl_energy[i];
        s_cl_1gev[i]    = cl_1gev[i];
        s_cl_2gev[i]    = cl_2gev[i];
        s_cl_bha[i]     = cl_bha[i];
      }
      s_ncl        = ncl;
      s_icn        = icn;
      s_icn_over   = icn_over;
      s_low_multi  = low_multi;
      s_b2bhabhav  = b2bhabhav;
      s_b2bhabhas  = b2bhabhas;
      s_mumu       = mumu;
      s_prescale   = prescale;
      s_b1bhabha   = b1bhabha;
      s_b1_type    = b1_type;
      s_physics    = physics;
      s_bg_veto    = bg_veto;
      s_etot       = etot;
      s_etot_type  = etot_type;
      s_ecl_bst    = ecl_bst;
      s_time       = time;
      s_time_type  = time_type;
    }
    /** the class title */
    ClassDef(TRGECLUnpackerSumStore, 4);
  };

} // end namespace Belle2

#endif
