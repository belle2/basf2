/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * DB object to store photon, hadron and diode shape parameters           *
 * per ECL crystal.                                                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vector>
#include <framework/logging/Logger.h>

/** DB object to store photon, hadron and diode shape parameters. */

namespace Belle2 {

  class ECLDigitWaveformParameters: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLDigitWaveformParameters() {};

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
