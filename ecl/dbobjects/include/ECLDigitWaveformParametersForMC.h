/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * DB object to store photon, hadron and diode shape parameters used in   *
 * simulations.  All crystals use same parameters in simulation.          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Savino Longo                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

/** DB object to store photon, hadron and diode shape parameters used in simulations.  All crystals use same parameters in simulation. */

namespace Belle2 {

  class ECLDigitWaveformParametersForMC: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLDigitWaveformParametersForMC()
    {
      for (int i = 0; i < 11; i++) {
        m_PhotonPars[i] = 0.;
        m_HadronPars[i] = 0.;
        m_DiodePars[i] = 0.;
      }

    };

    /** Get array of photon template parameters. 11 entries. */
    const float* getPhotonParameters() const {return m_PhotonPars;};

    /** Get array of hadron template parameters. 11 entries. */
    const float* getHadronParameters() const {return m_HadronPars;};

    /** Get array of diode template parameters. 11 entries. */
    const float* getDiodeParameters() const {return m_DiodePars;};

    /** Set photon, hadron and diode template parameters.  11 entries per template. One photon, hadron and diode template. */
    void setTemplateParameters(const float photonInput[11],
                               const float hadronInput[11],
                               const float diodeInput[11])
    {
      for (int i = 0; i < 11; i++) {
        m_PhotonPars[i] = photonInput[i];
        m_HadronPars[i] = hadronInput[i];
        m_DiodePars[i] = diodeInput[i];
      }
    };

    /**
     * Destructor
     */
    ~ECLDigitWaveformParametersForMC() {};

  private:

    float m_PhotonPars[11];  /**< photon parameters for MC*/
    float m_HadronPars[11];  /**< hadron parameters for MC*/
    float m_DiodePars[11];   /**< diode parameters for MC*/

    //1 Initial Version
    ClassDef(ECLDigitWaveformParametersForMC, 1); /**< ClassDef */
  };
} // end namespace Belle2
