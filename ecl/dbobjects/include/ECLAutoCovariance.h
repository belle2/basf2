/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Packed autocovariance database storage                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alexei Sibidanov                                         *
 *               Savino Longo                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <TObject.h>

/** Database storage for ECL autocovariance or noise spectral characteristics */

namespace Belle2 {

  /** packed covariance matrix */
  struct PackedAutoCovariance {
    float s2; /**< sigma noise squared in ADC channels */
    short int c[30]; /**< packed covariance, the range [-1.0, 1.0] is mapped to [-32767, 32767] */
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
      if (cellID < 1 || cellID > 8736) return m_acov[0];
      return m_acov[cellID - 1];
    }

    /** Get auto covariance for a channel */
    void getAutoCovariance(const int cellID, double acov[31]) const
    {
      if (cellID < 1 || cellID > 8736) return;
      const PackedAutoCovariance& t = m_acov[cellID - 1];
      acov[0] = static_cast<double>(t.s2);
      double norm = acov[0] * (1.0 / 32767);
      for (int i = 0; i < 30; i++) acov[i + 1] = norm * static_cast<double>(t.c[i]);
    }

    /** Set auto covariance for a channel */
    void setAutoCovariance(const int cellID, const double acov[31])
    {
      if (cellID < 1 || cellID > 8736) return;
      double norm = 32767 / acov[0];
      PackedAutoCovariance& t = m_acov[cellID - 1];
      t.s2 = static_cast<float>(acov[0]);
      for (int i = 0; i < 30; i++)
        t.c[i] = static_cast<short int>(std::max(-32767.0, std::min(acov[i + 1] * norm, 32767.0)));
    }

  private:
    PackedAutoCovariance m_acov[8736]; /**< Packed autocovariance matrix for each crystal */

    //Initial Version
    ClassDef(ECLAutoCovariance, 1); /**< ClassDef */
  };
} // end namespace Belle2
