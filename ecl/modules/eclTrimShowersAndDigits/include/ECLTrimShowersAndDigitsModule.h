/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/SelectSubset.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>


namespace Belle2 {
  class ECLCluster;
  class ECLShower;
  class ECLCalDigit;
  class MCParticle;


  /** Create new dataobjects containing only the ECLShower and ECLCalDigits
      associated with the selected ECLCluster, which is the one with maximum related
      MC energy. Also require the event have only one entry in MCParticles to get
      clean single photon events */
  class ECLTrimShowersAndDigitsModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ECLTrimShowersAndDigitsModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Event */
    virtual void event() override;

    /** Terminate */
    virtual void terminate() override;

  private:

    bool selectShower(const ECLShower* shower); /**< keep the ECLShower or not */
    bool selectDigit(const ECLCalDigit* digit); /**< keep the ECLCalDigit or not */

    StoreArray<ECLCluster> m_eclClusterArray;  /**< Array of ECLClusters */
    StoreArray<ECLShower> m_eclShowerArray; /**< Array of ECLShowers */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Array of ECLCalDigits */
    StoreArray<MCParticle> m_mcParticleArray; /**< Array of MCParticles */

    std::string m_ShowerArrayName = "ECLTrimmedShowers";  /**< Name of new ECLShower StoreArray */
    std::string m_DigitArrayName = "ECLTrimmedDigits";  /**< Name of new ECLCalDigit StoreArray */

    SelectSubset<ECLShower> m_selectedShowers; /**< selected ECLShowers */
    SelectSubset<ECLCalDigit> m_selectedDigits; /**< selected ECLCalDigits */

    unsigned short maxCellID = 9999; /**< used to identify the cluster of interest */

  };
}



