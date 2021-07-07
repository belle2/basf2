/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /** DB object to store photon, hadron and diode shape parameters. */
  class ECLDigitWaveformParameters: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLDigitWaveformParameters()
    {

      for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 8736; j++) {
          m_PhotonPars[j][i] = 0.;
          m_HadronPars[j][i] = 0.;
          m_DiodePars[j][i] = 0.;
        }
      }

    };

    /** Get array of photon template parameters. 11 entries per crystal. */
    const float* getPhotonParameters(int cellID) const {return m_PhotonPars[cellID - 1];};

    /** Get array of hadron template parameters. 11 entries per crystal. */
    const float* getHadronParameters(int cellID) const {return m_HadronPars[cellID - 1];};

    /** Get array of diode template parameters. 11 entries per crystal. */
    const float* getDiodeParameters(int cellID) const {return m_DiodePars[cellID - 1];};

    /** Set photon, hadron and diode template parameters for crystal. 11 entries per template. One photon, hadron and diode template per crystal. */
    void setTemplateParameters(int cellID, const float photonInput[11],
                               const float hadronInput[11],
                               const float diodeInput[11])
    {
      for (int i = 0; i < 11; i++) {
        m_PhotonPars[cellID - 1][i] = photonInput[i];
        m_HadronPars[cellID - 1][i] = hadronInput[i];
        m_DiodePars[cellID - 1][i] = diodeInput[i];
      }
    };

    /**
     * Destructor
     */
    ~ECLDigitWaveformParameters() {};

  private:
    float m_PhotonPars[8736][11];  /**< photon parameters all crystals*/
    float m_HadronPars[8736][11];  /**< hadron parameters all crystals*/
    float m_DiodePars[8736][11];   /**< diode parameters all crystals*/

    //2 convert vector of double to array of floats (SL)
    //1 Initial Version
    ClassDef(ECLDigitWaveformParameters, 2); /**< ClassDef */
  };
} // end namespace Belle2
