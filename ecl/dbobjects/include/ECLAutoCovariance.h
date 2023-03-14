/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* ECL headers. */
#include <ecl/dataobjects/ECLElementNumbers.h>

/* ROOT headers. */
#include <TObject.h>

/** Database storage for ECL autocovariance or noise spectral characteristics */

namespace Belle2 {

  /** packed covariance matrix */
  struct PackedAutoCovariance {
    float sigmaNoiseSq = -1; /**< sigma noise squared in ADC channels */
    short int packedCovMat[30] = {}; /**< packed covariance, the range [-1.0, 1.0] is mapped to [-32767, 32767] */
    ClassDef(PackedAutoCovariance, 1); /**< ClassDef to make streamer*/
  };

  /**
   *  Covariance matrices for offline ECL waveform fit
   */
  class ECLAutoCovariance: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLAutoCovariance() {};

    /**
     * Destructor
     */
    ~ECLAutoCovariance() {}

    /** Get packed autocovariance */
    const PackedAutoCovariance& getPacked(const int cellID) const
    {
      if (cellID < 1 || cellID > ECLElementNumbers::c_NCrystals) return m_acov[0];
      return m_acov[cellID - 1];
    }

    /** Get auto covariance for a channel */
    void getAutoCovariance(const int cellID, double acov[31]) const
    {
      if (cellID < 1 || cellID > ECLElementNumbers::c_NCrystals) return;
      const PackedAutoCovariance& tempPacked = m_acov[cellID - 1];
      acov[0] = static_cast<double>(tempPacked.sigmaNoiseSq);
      const double norm = acov[0] * (1.0 / 32767);
      for (int i = 0; i < 30; i++) acov[i + 1] = norm * static_cast<double>(tempPacked.packedCovMat[i]);
    }

    /** Set auto covariance for a channel */
    void setAutoCovariance(const int cellID, const double acov[31])
    {
      if (cellID < 1 || cellID > ECLElementNumbers::c_NCrystals) return;
      const double norm = 32767 / acov[0];
      PackedAutoCovariance& tempPacked = m_acov[cellID - 1];
      tempPacked.sigmaNoiseSq = static_cast<float>(acov[0]);
      for (int i = 0; i < 30; i++)
        tempPacked.packedCovMat[i] = static_cast<short int>(std::max(-32767.0, std::min(acov[i + 1] * norm, 32767.0)));
    }

  private:
    PackedAutoCovariance m_acov[ECLElementNumbers::c_NCrystals] = {}; /**< Packed autocovariance matrix for each crystal */

    //Initial Version
    ClassDef(ECLAutoCovariance, 1); /**< ClassDef */
  };
} // end namespace Belle2
