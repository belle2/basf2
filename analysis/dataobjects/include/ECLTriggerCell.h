/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  /** ECL Trigger cells */
  class ECLTriggerCell : public RelationsObject {

  public:

    /**
     * Default constructor.
     */
    ECLTriggerCell() :
      m_isHighestFADC(false),
      m_thetaId(0),
      m_phiId(0),
      m_TCId(0),
      m_hitWindow(std::numeric_limits<int>::quiet_NaN()),
      m_FADC(0.),
      m_timing(std::numeric_limits<double>::quiet_NaN()),
      m_evtTiming(std::numeric_limits<double>::quiet_NaN()),
      m_revoGDL(std::numeric_limits<double>::quiet_NaN()),
      m_revoFAM(std::numeric_limits<double>::quiet_NaN()),
      m_ECLCalDigitEnergy(0.)
    {}

    /** Set m_TCId*/
    void setTCId(unsigned int tcid) { m_TCId = tcid; }

    /** Set m_FADC*/
    void setFADC(float fadc) { m_FADC = fadc; }

    /** Set m_timingTC*/
    void setTiming(float timing) { m_timing = timing; }

    /** Set m_evtTimingTC*/
    void setEvtTiming(float evttiming) { m_evtTiming = evttiming; }

    /** Set m_revoGDLTC */
    void setRevoGDL(float revogdl) { m_revoGDL = revogdl; }

    /** Set m_revoFAM*/
    void setRevoFAM(float revofam) { m_revoFAM = revofam; }

    /** Set m_thetaId*/
    void setThetaId(unsigned int thetaid) { m_thetaId = thetaid; }

    /** Set m_phiId*/
    void setPhiId(unsigned int phiid) { m_phiId = phiid; }

    /** Set m_ECLCalDigitEnergy*/
    void setECLCalDigitEnergy(float energy) { m_ECLCalDigitEnergy = energy; }

    /** Set m_isHighestFADC*/
    void setIsHighestFADC(bool ishighest) { m_isHighestFADC = ishighest; }

    /** Set m_hitWindow*/
    void setHitWin(int hitwin) { m_hitWindow = hitwin; }


    /** Get m_TCId */
    unsigned int getTCId() const
    {
      return m_TCId;
    }

    /** Get m_FADC */
    float getFADC() const
    {
      return m_FADC;
    }

    /** Get m_timing */
    float getTiming() const
    {
      return m_timing;
    }

    /** Get m_evtTiming */
    float getEvtTiming() const
    {
      return m_evtTiming;
    }

    /** Get m_revoGDL */
    float getRevoGDL() const
    {
      return m_revoGDL;
    }

    /** Get m_revoFAM */
    float getRevoFAM() const
    {
      return m_revoFAM;
    }

    /** Get m_thetaId */
    float getThetaId() const
    {
      return m_thetaId;
    }

    /** Get m_phiId */
    float getPhiId() const
    {
      return m_phiId;
    }

    /** Get m_ECLCalDigitEnergy */
    float getECLCalDigitEnergy() const
    {
      return m_ECLCalDigitEnergy;
    }

    /** Get m_isHighestFADC */
    bool isHighestFADC() const
    {
      return m_isHighestFADC;
    }

    /** Get m_hitWindow */
    int getHitWin() const
    {
      return m_hitWindow;
    }

  private:

    bool m_isHighestFADC; /**< true if this TC has the highest FADC value in the event */
    unsigned int m_thetaId; /**<theta id for this TC*/
    unsigned int m_phiId; /**<phi id for this TC*/
    unsigned int m_TCId; /**<TC Id (1..576)*/
    int m_hitWindow; /**< hit window of this TC */
    float m_FADC; /**<FADC for this TC  (flash analogue-to-digital)*/
    float m_timing; /**<timing for this TC*/
    float m_evtTiming; /**<event timing for this TC*/
    float m_revoGDL; /**<revoGDL for this TC (revolution global decision logic)*/
    float m_revoFAM; /**<revoFAM for this TC*/
    float m_ECLCalDigitEnergy; /**<sum of all ECLCalDigits for this TC*/

    ClassDef(ECLTriggerCell, 2); /**< class definition */

  };

} // end namespace Belle2
