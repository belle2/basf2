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

namespace Belle2 {
  /**
   *Clustering event level information, for example out of time ECLCalDigits, or ECLClusters rejected before storing to mdst.
   */
  class EventLevelClusteringInfo : public TObject {
  public:

    /** Getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), FWD only. */
    uint16_t getNECLCalDigitsOutOfTimeFWD() const
    {
      return m_nECLCalDigitsOutOfTimeFWD;
    }

    /** Getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), Barrel only */
    uint16_t getNECLCalDigitsOutOfTimeBarrel() const
    {
      return m_nECLCalDigitsOutOfTimeBarrel;
    }

    /** Getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), BWD only */
    uint16_t getNECLCalDigitsOutOfTimeBWD() const
    {
      return m_nECLCalDigitsOutOfTimeBWD;
    }

    /** Getter for the number of ECLCalDigits that are out of time and above some MeV (scale ~linearly with background), FWD+Barrel+BWD */
    uint16_t getNECLCalDigitsOutOfTime() const
    {
      return m_nECLCalDigitsOutOfTimeFWD + m_nECLCalDigitsOutOfTimeBarrel + m_nECLCalDigitsOutOfTimeBWD;
    }

    /** Setter for the number of ECLCalDigits that are out of time and above some MeV, FWD only. */
    void setNECLCalDigitsOutOfTimeFWD(uint16_t const nECLCalDigitsOutOfTimeFWD)
    {
      m_nECLCalDigitsOutOfTimeFWD = nECLCalDigitsOutOfTimeFWD;
    }

    /** Setter for the number of ECLCalDigits that are out of time and above some MeV, Barrel only. */
    void setNECLCalDigitsOutOfTimeBarrel(uint16_t const nECLCalDigitsOutOfTimeBarrel)
    {
      m_nECLCalDigitsOutOfTimeBarrel = nECLCalDigitsOutOfTimeBarrel;
    }

    /** Setter for the number of ECLCalDigits that are out of time and above some MeV, BWD only. */
    void setNECLCalDigitsOutOfTimeBWD(uint16_t const nECLCalDigitsOutOfTimeBWD)
    {
      m_nECLCalDigitsOutOfTimeBWD = nECLCalDigitsOutOfTimeBWD;
    }

    /** Getter for the number of photon ECLShowers that are not stored as ECLClusters, FWD. */
    uint8_t getNECLShowersRejectedFWD() const
    {
      return m_nECLShowersRejectedFWD;
    }

    /** Getter for the number of photon ECLShowers that are not stored as ECLClusters, Barrel. */
    uint8_t getNECLShowersRejectedBarrel() const
    {
      return m_nECLShowersRejectedBarrel;
    }

    /** Getter for the number of photon ECLShowers that are not stored as ECLClusters, BWD. */
    uint8_t getNECLShowersRejectedBWD() const
    {
      return m_nECLShowersRejectedBWD;
    }

    /** Getter for the number of photon ECLShowers that are not stored as ECLClusters. */
    uint8_t getNECLShowersRejected() const
    {
      return m_nECLShowersRejectedFWD + m_nECLShowersRejectedBarrel + m_nECLShowersRejectedBWD;
    }

    /** Setter for the number of photon ECLShowers that are not stored as ECLClusters, FWD. */
    void setNECLShowersRejectedFWD(uint8_t const nECLShowersRejectedFWD)
    {
      m_nECLShowersRejectedFWD = nECLShowersRejectedFWD;
    }

    /** Setter for the number of photon ECLShowers that are not stored as ECLClusters, Barrel. */
    void setNECLShowersRejectedBarrel(uint8_t const nECLShowersRejectedBarrel)
    {
      m_nECLShowersRejectedBarrel = nECLShowersRejectedBarrel;
    }

    /** Setter for the number of photon ECLShowers that are not stored as ECLClusters, BWD. */
    void setNECLShowersRejectedBWD(uint8_t const nECLShowersRejectedBWD)
    {
      m_nECLShowersRejectedBWD = nECLShowersRejectedBWD;
    }

  private:
    /** Number of out of time, energetic ECLCalDigits, FWD. */
    uint16_t m_nECLCalDigitsOutOfTimeFWD {0};

    /** Number of out of time, energetic ECLCalDigits, Barrel. */
    uint16_t m_nECLCalDigitsOutOfTimeBarrel {0};

    /** Number of out of time, energetic ECLCalDigits, BWD. */
    uint16_t m_nECLCalDigitsOutOfTimeBWD {0};

    /** Number of photon showers that are rejected before storing to mdst (max. 255), FWD. */
    uint8_t m_nECLShowersRejectedFWD {0};

    /** Number of photon showers that are rejected before storing to mdst (max. 255), Barrel. */
    uint8_t m_nECLShowersRejectedBarrel {0};

    /** Number of photon showers that are rejected before storing to mdst (max. 255), BWD. */
    uint8_t m_nECLShowersRejectedBWD {0};

    ClassDef(EventLevelClusteringInfo, 1); /**< ROOT. */
  };
}
