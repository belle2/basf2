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
#include <vector>
#include <framework/logging/Logger.h>

/** DB object to store photon, hadron and diode shape parameters used in simulations.  All crystals use same parameters in simulation. */

namespace Belle2 {

  class ECLDigitWaveformParametersForMC: public TObject {
  public:

    /**
     * Default constructor
     */
    ECLDigitWaveformParametersForMC() {};

    /** Get vector of photon template parameters. 11 entries. */
    const std::vector<float>& getPhotonParameters() const {return m_PhotonPars;};

    /** Get vector of hadron template parameters. 11 entries. */
    const std::vector<float>& getHadronParameters() const {return m_HadronPars;};

    /** Get vector of diode template parameters. 11 entries. */
    const std::vector<float>& getDiodeParameters() const {return m_DiodePars;};

    /** Set photon, hadron and diode template parameters.  11 entries per template. One photon, hadron and diode template. */
    void setTemplateParameters(const std::vector<float>& photonInput,
                               const std::vector<float>& hadronInput,
                               const std::vector<float>& diodeInput)
    {
      if (photonInput.size() != 11 || hadronInput.size() != 11 || diodeInput.size() != 11) {B2FATAL("ECLDigitWaveformParametersForMC: wrong size vector " << photonInput.size() << " " << hadronInput.size() << " " << diodeInput.size() << " instead of 11");}
      m_PhotonPars = photonInput;
      m_HadronPars = hadronInput;
      m_DiodePars = diodeInput;
    };

    /**
     * Destructor
     */
    ~ECLDigitWaveformParametersForMC() {};

  private:
    std::vector<float> m_PhotonPars;  /**< photon parameters for MC*/
    std::vector<float> m_HadronPars;  /**< hadron parameters for MC*/
    std::vector<float> m_DiodePars;   /**< diode parameters for MC*/

    //1 Initial Version
    ClassDef(ECLDigitWaveformParametersForMC, 1); /**< ClassDef */
  };
} // end namespace Belle2
