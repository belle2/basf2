/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Matic Lubej                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TLorentzVector.h>
#include <map>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Returns 1 if a track, ecl or klmCluster associated to particle is in the related RestOfEvent object, 0 otherwise
     */
    double isInRestOfEvent(const Particle* particle);

    /**
     * Returns number of all the tracks in the related RestOfEvent object
     */
    double nAllROETracks(const Particle* particle);

    /**
     * Returns number of tracks in the related RestOfEvent object that pass the selection criteria
     */
    double nROETracks(const Particle* particle);

    /**
     * Returns number of tracks in the event minus in the current RestOfEvent object
     */
    double nRemainingTracksInRestOfEvent(const Particle* particle);

    /**
     * Returns number of all ECL clusters in the related RestOfEvent object
     */
    double nAllROEECLClusters(const Particle* particle);

    /**
     * Returns number of ECL clusters in the related RestOfEvent object that pass the selection criteria
     */
    double nROEECLClusters(const Particle* particle);

    /**
     * Returns number of remaining KLM clusters in the related RestOfEvent object
     */
    double nROEKLMClusters(const Particle* particle);

    /**
     * Returns 1 if the invariant mass of a combination of a photon in RestOfEvent with
     * the signal photon yields the mass of the a neutral Pion.
     */
    double pionVeto(const Particle* particle);

    /**
     * Returns number of all neutral ECL clusters in the related RestOfEvent object
     */
    double nAllROENeutralECLClusters(const Particle* particle);

    /**
     * Returns number of neutral ECL clusters in the related RestOfEvent object that pass the selection criteria
     */
    double nROENeutralECLClusters(const Particle* particle);

    /**
     * Returns number of lepton particles in the related RestOfEvent object (counts particles and anti-particles)
     */
    double nROELeptons(const Particle* particle);

    /**
     * Returns total charge of the related RestOfEvent object
     */
    double ROECharge(const Particle* particle);

    /**
     * return extra energy in the calorimeter that is not associated to the given Particle
     */
    double extraEnergy(const Particle* particle);

    /**
     * return extra energy in the calorimeter that is not associated to the given Particle
     * ECLClusters passing goodGamma selection are used only.
     */
    double extraEnergyFromGoodGamma(const Particle* particle);

    /**
     * return extra energy in the calorimeter that is not associated to the given Particle
     * ECLClusters passing goodBelleGamma (Belle 1 criteria) selection are used only.
     */
    double extraEnergyFromGoodBelleGamma(const Particle* particle);

    /**
     * Returns energy difference of the related RestOfEvent object with respect to E_cms/2
     */
    double ROEDeltaE(const Particle* particle);

    /**
     * Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2
     */
    double ROEMbc(const Particle* particle);

    /**
     * Returns MC Errors for an artificial tag side particle, which corresponds to the ROE object
     */
    double ROEMCErrors(const Particle* particle);

    /**
     * Returns the energy difference of the B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2
     */
    double correctedBMesonDeltaE(const Particle* particle);

    /**
     * Returns beam constrained mass of B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2
     */
    double correctedBMesonMbc(const Particle* particle);

    /**
     * Returns the missing mass squared.
     * Option 0: Take momentum and energy of all ROE tracks and clusters into account
     * Option 1: Take only momentum of ROE tracks and clusters into account, energy set to E_beam
     * Option 2: Don't take any ROE tracks and clusters into account, use signal side only
     * Option 3: Same as option 2, but use the correction of the B meson momentum magnitude in LAB
     *           system in the direction of the ROE momentum
     */
    double ROEMissingMass(const Particle* particle, const std::vector<double>& opt);

    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    /**
     * Returns the missing 4-momentum vector in CMS system.
     * Option 0: Take momentum and energy of all ROE tracks and clusters into account
     * Option 1: Take only momentum of ROE tracks and clusters into account, energy set to E_beam
     * Option 2: Don't take any ROE tracks and clusters into account, use signal side only
     * Option 3: Same as option 2, but use the correction of the B meson momentum magnitude in LAB
     *           system in the direction of the ROE momentum
     */
    TLorentzVector missing4VectorCMS(const Particle* particle, const std::vector<double>& opt);

    /**
     * Returns the neutrino 4-momentum vector in CMS system. Mass of neutrino is 0 by definition: E == |p|
     */
    TLorentzVector neutrino4VectorCMS(const Particle* particle);

  }
} // Belle2 namespace

