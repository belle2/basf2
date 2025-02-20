/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// Root
#include <TObject.h>

// basf2
#include <analysis/dbobjects/ParticleWeightingLookUpTable.h>

namespace Belle2 {

  /**
   * Bin holder as vector for bin limit pairs: [energy limits, theta limits, phi limits]
   */
  typedef std::vector<std::pair<double, double>> Binning;

  /**
   * Class to hold the information ECL energy resolution derived from PERC.
   * Currently the energy resolution is given in energy bins and only valid for barrel region
   */
  class ECLPhotonEnergyResolution : public TObject {
  public:
    /**!
     * Default constructor
     */
    ECLPhotonEnergyResolution();

    /**!
     * Destructor
     */
    ~ECLPhotonEnergyResolution() {};

    /*!
     * Get ParticleWeightingLookUpTable with binned energy resolution
     * @return ParticleWeightingLookUpTable of the energy resolution in energy, theta, and phi bins
     */
    ParticleWeightingLookUpTable getFullEnergyResolution() const { return m_resolutionBinningTable; }

    /*!
     * Add energy bin [vector of three pairs] and energy resolution [vector of three values]
     * Energy resolution structure: [value, uncertainty up, uncertainty down]
     * Energy bin structure for bin limit pairs: [energy limits, theta limits, phi limits]
     * @param energyResolution energy resolution with uncertainties
     * @param binning binning of energy resolution valid region in bins of [energy, theta phi]
     */
    void addRelativeEnergyResolution(std::vector<double> energyResolution, Binning binning);

    /*!
     * Get energy resolution for given energy, theta, phi
     * @param energy photon energy value
     * @param theta photon theta value
     * @param phi photon phi value
     * @return Energy resolution
     */
    double getRelativeEnergyResolution(double energy, double theta, double phi) const;

    /*!
     * For neutral hadrons
     * Add theta resolution [vector of three values] and phi resolution [vector of three values] in bins of (clusterUncEnergy, theta, phi) [vector of three pairs]
     * thetaResolution structure: [value, uncertainty up, uncertainty down]
     * phiResolution structure: [value, uncertainty up, uncertainty down]
     * bin structure for bin limit pairs: [energy limits, theta limits, phi limits]
     * @param thetaResolution theta resolution with uncertainties
     * @param phiResolution phi resolution with uncertainties
     * @param binning binning of theta/phi resolution valid region in bins of [energy, theta phi]
     */
    void addThetaPhiResolution(std::vector<double> thetaResolution, std::vector<double> phiResolution, Binning binning);

    /*!
     * Get theta or phi resolution for given energy, theta, phi
     * @param energy - for photons, energy value, for hadrons clusterUnCorr energy value
     * @param theta photon theta value
     * @param phi photon phi value
     * @param thetaOrPhi flag to pick theta or phi, 0 - return theta resolution, 1 - return phi resolution
     * @return Theta or Phiresolution
     */
    double getThetaPhiResolution(double energy, double theta, double phi, bool thetaOrPhi) const;


  private:
    /*!
     * Utilise ParticleWeighting modules as holders for binning of energy resolution
     */
    ParticleWeightingLookUpTable m_resolutionBinningTable;

    // 1: Initial version
    // 2: Added theta phi resolution functionality for neutral hadrons
    ClassDef(ECLPhotonEnergyResolution, 2); /**< ClassDef */
  };
} // end namespace Belle2
