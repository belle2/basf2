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

/* Basf2 headers. */
#include <calibration/CalibrationCollectorModule.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ECLCluster;
  class ECLShower;
  class ECLCalDigit;
  class MCParticle;

  namespace ECL {
    class ECLNeighbours;
  }

  /** Collector that runs on single photon MC samples to find the number
   * of crystals to be summed to get the best energy resolution for each
   * test energy and for each group of crystals (8 groups per thetaID in
   * the barrel). Also finds the corresponding energy bias from beam
   * backgrounds, and the peak fraction of energy contained in the crystals.
   *
   * Contact Chris Hearty hearty@physics.ubc.ca for questions or concerns   */

  /** Calibration collector module that uses single photon MC sample to find optimal number of crystals to be used in cluster energy sum */
  class eclNOptimalCollectorModule : public CalibrationCollectorModule {

  public:

    /** Constructor: Sets the description, the properties and the parameters of the module */
    eclNOptimalCollectorModule();

    /** Define histograms */
    void prepare() override;

    /** Select events and crystals and accumulate histograms */
    void collect() override;

  private:

    /** Parameters */
    int m_numberEnergies; /**< number of generated energies (8) */
    std::vector<double> m_energiesForward; /**< generated photon energies, forward */
    std::vector<double> m_energiesBarrel; /**< generated photon energies, barrel */
    std::vector<double> m_energiesBackward; /**< generated photon energies, backward */

    std::string m_digitArrayName;  /**< Name of ECLCalDigit StoreArray */
    std::string m_showerArrayName;  /**< Name of ECLShower StoreArray */

    int m_nGroupPerThetaID; /**< number of groups per standard thetaID */

    /** Some other useful quantities */
    std::vector< std::vector<int> > iEnergies;/**< Generated energies in MeV in each region */
    const int iFirstCellId = 161; /**< first useful cellID (first of thetaID 3) */
    const int iLastCellId = 8608; /**< first useful cellID (last of thetaID 66) */
    const int nLeakReg = 3; /**< 0 = forward, 1 = barrel, 2 = backward */
    const int nCrysMax = 21; /**< max number of crystals used to calculate energy */

    int nCrystalGroups; /**< sort the crystals into this many groups */
    int iGroupOfCrystal[ECLElementNumbers::c_NCrystals]; /**< group number of each crystal */

    ECL::ECLNeighbours* neighbours{nullptr}; /**< neighbours to crystal */

    bool storeParameters = true; /**< store parameters first event */

    /** Required arrays */
    StoreArray<ECLShower> m_eclShowerArray; /**< Array of ECLShowers */
    StoreArray<ECLCluster> m_eclClusterArray; /**< Array of ECLClusters */
    StoreArray<ECLCalDigit> m_eclCalDigitArray; /**< Array of ECLCalDigits */
    StoreArray<MCParticle> m_mcParticleArray; /**< Array of MCParticles */
  };
}
