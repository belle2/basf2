/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** ECL Trigger cells */
  class ECLTC : public RelationsObject {

  public:

    /**
     * Default constructor.
     */
    ECLTC() :
      m_TCId(0),
      m_FADC(0.),
      m_timing(std::numeric_limits<double>::quiet_NaN()),
      m_evtTiming(std::numeric_limits<double>::quiet_NaN()),
      m_revoGDL(std::numeric_limits<double>::quiet_NaN()),
      m_revoFAM(std::numeric_limits<double>::quiet_NaN()),
      m_thetaId(0),
      m_phiId(0),
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

    /** Set m_revoGDLTC*/
    void setRevoGDL(float revogdl) { m_revoGDL = revogdl; }

    /** Set m_revoFAM*/
    void setRevoFAM(float revofam) { m_revoFAM = revofam; }

    /** Set m_thetaId*/
    void setThetaId(unsigned int thetaid) { m_thetaId = thetaid; }

    /** Set m_phiId*/
    void setPhiId(unsigned int phiid) { m_phiId = phiid; }

    /** Set m_ECLCalDigitEnergy*/
    void setECLCalDigitEnergy(float energy) { m_ECLCalDigitEnergy = energy; }


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

  private:

    unsigned int m_TCId; /**<TC Id (1..576)*/
    float m_FADC; /**<FADC for this TC*/
    float m_timing; /**<timing for this TC*/
    float m_evtTiming; /**<event timing for this TC*/
    float m_revoGDL; /**<revoGDL for this TC*/
    float m_revoFAM; /**<revoFAM for this TC*/
    unsigned int m_thetaId; /**<theta id for this TC*/
    unsigned int m_phiId; /**<phi id for this TC*/
    float m_ECLCalDigitEnergy; /**<sum of all ECLCalDigits for this TC*/

    ClassDef(ECLTC, 1); /**< class definition */

  };

} // end namespace Belle2
