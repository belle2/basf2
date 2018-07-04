/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Class to store information about ECL trigger cells */
  class ECLTRGInformation : public TObject {

  public:

    /**
     * Default constructor.
     */
    ECLTRGInformation() :
      m_thetaIdTC(576),
      m_phiIdTC(576),
      m_energyTC(576),
      m_timingTC(576),
      m_revoGDLTC(576),
      m_revoFAMTC(576),
      m_energyTCECLCalDigit(576),
      m_timingTCECLCalDigit(576)
    {}

    /** Set m_thetaIdTC */
    void setThetaIdTC(const int& tcid, const int& tcthetaid)
    {
      if (tcid >= 0 and tcid < 576) {
        m_thetaIdTC[tcid] = tcthetaid;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_phiIdTC */
    void setPhiIdTC(const int& tcid, const int& tcphiid)
    {
      if (tcid >= 0 and tcid < 576) {
        m_phiIdTC[tcid] = tcphiid;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_energyTC */
    void setEnergyTC(const int& tcid, const float& tcenergy)
    {
      if (tcid >= 0 and tcid < 576) {
        m_energyTC[tcid] = tcenergy;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_timingTC */
    void setTimingTC(const int& tcid, const float& tctiming)
    {
      if (tcid >= 0 and tcid < 576) {
        m_timingTC[tcid] = tctiming;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_revoGDLTC */
    void setRevoGDLTC(const int& tcid, const float& tcrevotrg)
    {
      if (tcid >= 0 and tcid < 576) {
        m_revoGDLTC[tcid] = tcrevotrg;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_revoFAMTC */
    void setRevoFAMTC(const int& tcid, const float& tcrevofam)
    {
      if (tcid >= 0 and tcid < 576) {
        m_revoFAMTC[tcid] = tcrevofam;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Get m_phiIdTC */
    int getPhiIdTC(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_phiIdTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_thetaIdTC */
    int getThetaIdTC(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_thetaIdTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_energyTC */
    float getEnergyTC(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_energyTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_timingTC */
    float getTimingTC(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_timingTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_revoGDLTC */
    float getRevoTRGTC(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_revoGDLTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_revoFAMTC */
    float getRevoFAMTC(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_revoFAMTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Set m_energyTCECLCalDigit */
    void setEnergyTCECLCalDigit(const int& tcid, const float& tcenergy)
    {
      if (tcid >= 0 and tcid < 576) {
        m_energyTCECLCalDigit[tcid] = tcenergy;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Get m_energyTCECLCalDigit */
    float getEnergyTCECLCalDigit(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_energyTCECLCalDigit[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Set m_timingTCECLCalDigit */
    void setTimingTCECLCalDigit(const int& tcid, const float& tctiming)
    {
      if (tcid >= 0 and tcid < 576) {
        m_timingTCECLCalDigit[tcid] = tctiming;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Get m_timingTCECLCalDigit */
    float getTimingTCECLCalDigit(const int& tcid)
    {
      if (tcid >= 0 and tcid < 576) {
        return m_timingTCECLCalDigit[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

  private:

    std::vector<int>
    m_thetaIdTC; /**<thetaid, one entry per ECL trigger cell (TC) - this is a constant quantity, no actual need to store if for every event*/
    std::vector<int>
    m_phiIdTC; /**<phiid, one entry per ECL trigger cell (TC)  - this is a constant quantity, no actual need to store if for every event*/
    std::vector<float> m_energyTC; /**<energy, one entry per ECL trigger cell (TC) */
    std::vector<float> m_timingTC; /**<timing, one entry per ECL trigger cell (TC) */
    std::vector<float> m_revoGDLTC; /**<revogdl, one entry per ECL trigger cell (TC) */
    std::vector<float> m_revoFAMTC; /**<revofam, one entry per ECL trigger cell (TC) */

    std::vector<float> m_energyTCECLCalDigit; /**<energy, one entry per ECL trigger cell (TC) based on ECLCalDigits*/
    std::vector<float>
    m_timingTCECLCalDigit; /**<timing (of highest energy caldigit), one entry per ECL trigger cell (TC) based on ECLCalDigits*/

    ClassDef(ECLTRGInformation, 1); /**< class definition */

  };

} // end namespace Belle2
