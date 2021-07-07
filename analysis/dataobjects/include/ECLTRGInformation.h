/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Class to store information about ECL trigger cells (TCs) */
  class ECLTRGInformation : public TObject {

  public:

    /** Number of TCs */
    static constexpr int c_nTCs = 576;

    /**
     * Default constructor.
     */
    ECLTRGInformation() :
      m_thetaIdTC(c_nTCs + 1),
      m_phiIdTC(c_nTCs + 1),
      m_energyTC(c_nTCs + 1),
      m_timingTC(c_nTCs + 1),
      m_revoGDLTC(c_nTCs + 1),
      m_revoFAMTC(c_nTCs + 1),
      m_hitWinTC(c_nTCs + 1),
      m_energyTCECLCalDigit(c_nTCs + 1),
      m_timingTCECLCalDigit(c_nTCs + 1),
      m_clusterEnergyThreshold(0.),
      m_sumEnergyTCECLCalDigitInECLCluster(0.),
      m_sumEnergyECLCalDigitInECLCluster(0.),
      m_evtTiming(std::numeric_limits<float>::quiet_NaN()),
      m_maximumTCId(-1)
    {
      // some vectors should not be initialized with zeroes but with NaN
      for (unsigned idx = 0; idx <= c_nTCs; idx++) {
        m_timingTC[idx] = std::numeric_limits<float>::quiet_NaN();
        m_timingTCECLCalDigit[idx] = std::numeric_limits<float>::quiet_NaN();
        m_hitWinTC[idx] = std::numeric_limits<int>::quiet_NaN();
      }
    }

    /** Set m_thetaIdTC */
    void setThetaIdTC(const int& tcid, const int& tcthetaid)
    {
      if (tcid > 0 and tcid <= c_nTCs + 1) {
        m_thetaIdTC[tcid] = tcthetaid;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_phiIdTC */
    void setPhiIdTC(const int& tcid, const int& tcphiid)
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        m_phiIdTC[tcid] = tcphiid;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_energyTC */
    void setEnergyTC(const int& tcid, const float& tcenergy)
    {
      if (tcid >= 1 and tcid < c_nTCs + 1) {
        m_energyTC[tcid] = tcenergy;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_timingTC */
    void setTimingTC(const int& tcid, const float& tctiming)
    {
      if (tcid >= 1 and tcid < c_nTCs + 1) {
        m_timingTC[tcid] = tctiming;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_evtTiming */
    void setEvtTiming(float evttiming) { m_evtTiming = evttiming; }

    /** Set m_revoGDLTC */
    void setRevoGDLTC(const int& tcid, const float& tcrevotrg)
    {
      if (tcid >= 1 and tcid < c_nTCs + 1) {
        m_revoGDLTC[tcid] = tcrevotrg;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_revoFAMTC */
    void setRevoFAMTC(const int& tcid, const float& tcrevofam)
    {
      if (tcid >= 1 and tcid < c_nTCs + 1) {
        m_revoFAMTC[tcid] = tcrevofam;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Set m_hitWinTC */
    void setHitWinTC(const int& tcid, const int& hitwin)
    {
      if (tcid >= 1 and tcid < c_nTCs + 1) {
        m_hitWinTC[tcid] = hitwin;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Get m_phiIdTC */
    int getPhiIdTC(const int& tcid) const
    {
      if (tcid >= 1 and tcid < c_nTCs + 1) {
        return m_phiIdTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_thetaIdTC */
    int getThetaIdTC(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_thetaIdTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_energyTC */
    float getEnergyTC(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_energyTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_timingTC */
    float getTimingTC(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_timingTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_revoGDLTC */
    float getRevoGDLTC(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_revoGDLTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_revoFAMTC */
    float getRevoFAMTC(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_revoFAMTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Get m_hitWinTC */
    float getHitWinTC(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_hitWinTC[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Set m_energyTCECLCalDigit */
    void setEnergyTCECLCalDigit(const int& tcid, const float& tcenergy)
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        m_energyTCECLCalDigit[tcid] = tcenergy;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Get m_energyTCECLCalDigit */
    float getEnergyTCECLCalDigit(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_energyTCECLCalDigit[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Set m_timingTCECLCalDigit */
    void setTimingTCECLCalDigit(const int& tcid, const float& tctiming)
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        m_timingTCECLCalDigit[tcid] = tctiming;
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
      }
    }

    /** Get m_timingTCECLCalDigit */
    float getTimingTCECLCalDigit(const int& tcid) const
    {
      if (tcid > 0 and tcid < c_nTCs + 1) {
        return m_timingTCECLCalDigit[tcid];
      } else {
        B2ERROR("TC " << tcid << " does not exist.");
        return 0.;
      }
    }

    /** Set m_clusterEnergyThreshold*/
    void setClusterEnergyThreshold(float thresh) { m_clusterEnergyThreshold = thresh; }

    /** Get m_clusterEnergyThreshold */
    float getClusterEnergyThreshold() const
    {
      return m_clusterEnergyThreshold;
    }

    /** Set m_sumEnergyTCECLCalDigitInECLCluster*/
    void setSumEnergyTCECLCalDigitInECLCluster(float sumenergy) { m_sumEnergyTCECLCalDigitInECLCluster = sumenergy; }

    /** Get m_clusterEnergyThreshold */
    float getSumEnergyTCECLCalDigitInECLCluster() const
    {
      return m_sumEnergyTCECLCalDigitInECLCluster;
    }

    /** Set m_sumEnergyECLCalDigitInECLCluster*/
    void setSumEnergyECLCalDigitInECLCluster(float sumenergy) { m_sumEnergyECLCalDigitInECLCluster = sumenergy; }

    /** Get m_clusterEnergyThreshold */
    float getSumEnergyECLCalDigitInECLCluster() const
    {
      return m_sumEnergyECLCalDigitInECLCluster;
    }

    /** Get m_evtTiming */
    float getEvtTiming() const
    {
      return m_evtTiming;
    }

    /** Set m_maximumTCId*/
    void setMaximumTCId(int maxtcid) { m_maximumTCId = maxtcid; }

    /** Get m_maximumTCId */
    int getMaximumTCId() const
    {
      return m_maximumTCId;
    }

  private:

    std::vector<int>
    m_thetaIdTC; /**<thetaid, one entry per ECL TC - this is a constant quantity, no actual need to store it for every event*/
    std::vector<int>
    m_phiIdTC; /**<phiid, one entry per ECL TC  - this is a constant quantity, no actual need to store if for every event*/
    std::vector<float> m_energyTC; /**<energy, one entry per ECL TC */
    std::vector<float> m_timingTC; /**<timing, one entry per ECL TC */
    std::vector<float> m_revoGDLTC; /**<revogdl, one entry per ECL TC */
    std::vector<float> m_revoFAMTC; /**<revofam, one entry per ECL TC */
    std::vector<int> m_hitWinTC; /**<hitwindow, one entry per ECL TC */

    std::vector<float> m_energyTCECLCalDigit; /**<energy, one entry per ECL TC based on ECLCalDigits*/
    std::vector<float>
    m_timingTCECLCalDigit; /**<timing (of highest energy eclcaldigit), one entry per ECL TC based on ECLCalDigits*/

    float m_clusterEnergyThreshold; /**<energy threshold for clusters to be included in m_sumEnergyTCECLCalDigitInECLCluster*/
    float m_sumEnergyTCECLCalDigitInECLCluster; /**<sum of energy in ECL TCs based on ECLCalDigits that are part of an ECLCluster above threshold*/
    float m_sumEnergyECLCalDigitInECLCluster; /**<sum of energy based on ECLCalDigits that are part of an ECLCluster above threshold*/
    float m_evtTiming; /**<TC evttime, one entry per event */
    int m_maximumTCId; /**<TC Id of TC with maximum FADC count */

    ClassDef(ECLTRGInformation, 3); /**< class definition */

  };

} // end namespace Belle2
