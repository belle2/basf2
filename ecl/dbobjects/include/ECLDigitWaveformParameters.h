/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
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

    /** Get vector of photon template parameters. 11 entries per crystal. */
    const std::vector<double>& getPhotonParameters(int cellID) const {return m_PhotonPars[cellID - 1];};

    /** Get vector of hadron template parameters. 11 entries per crystal. */
    const std::vector<double>& getHadronParameters(int cellID) const {return m_HadronPars[cellID - 1];};

    /** Get vector of diode template parameters. 11 entries per crystal. */
    const std::vector<double>& getDiodeParameters(int cellID) const {return m_DiodePars[cellID - 1];};

    /** Set photon, hadron and diode template parameters for crystal. 11 entries per template. One photon, hadron and diode template per crystal. */
    void setTemplateParameters(int cellID, const std::vector<double>& photonInput,
                               const std::vector<double>& hadronInput,
                               const std::vector<double>& diodeInput)
    {
      if (photonInput.size() != 11 || hadronInput.size() != 11 || diodeInput.size() != 11) {B2FATAL("ECLDigitWaveformParameters: wrong size vector " << photonInput.size() << " " << hadronInput.size() << " " << diodeInput.size() << " instead of 11");}
      if (m_PhotonPars.size() == 0)  m_PhotonPars.resize(8736);
      if (m_HadronPars.size() == 0)  m_HadronPars.resize(8736);
      if (m_DiodePars.size() == 0)  m_DiodePars.resize(8736);
      m_PhotonPars[cellID - 1] = photonInput;
      m_HadronPars[cellID - 1] = hadronInput;
      m_DiodePars[cellID - 1] = diodeInput;
    };

    /**
     * Destructor
     */
    ~ECLDigitWaveformParameters() {};

  private:
    std::vector< std::vector<double> > m_PhotonPars;  /**< photon parameters all crystals*/
    std::vector< std::vector<double> > m_HadronPars;  /**< hadron parameters all crystals*/
    std::vector< std::vector<double> > m_DiodePars;   /**< diode parameters all crystals*/

    //1 Initial Version
    ClassDef(ECLDigitWaveformParameters, 1); /**< ClassDef */
  };
} // end namespace Belle2
