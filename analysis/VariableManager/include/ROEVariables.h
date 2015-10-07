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

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Returns 1 if a track, ecl or klmCluster associated to particle is in the related RestOfEvent object, 0 otherwise
     */
    double isInRestOfEvent(const Particle* particle);

    /**
     * Returns number of tracks in the related RestOfEvent object
     */
    double nROETracks(const Particle* particle);

    /**
     * Returns number of tracks in the event minus in the current RestOfEvent object
     */
    double nRemainingTracksInRestOfEvent(const Particle* particle);

    /**
     * Returns number of ECL clusters in the related RestOfEvent object
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
     * Returns number of remaining neutral ECL clusters in the related RestOfEvent object
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
     * Returns energy difference of the signal side (reconstructed side + neutrino) with respect to E_cms/2
     */
    double correctedDeltaE(const Particle* particle);

    /**
     * Returns beam constrained mass of the signal side (reconstructed side + neutrino) with respect to E_cms/2
     */
    double correctedMbc(const Particle* particle);

    /**
     * Returns the missing mass squared. Two definitions exist:
     * Option 0: (E)vent based missing mass: calculates the missing 4-momentum based on all the momentum and energy in the EVENT
     * Option 1: (C)andidate based missing mass: calculates the missing 4-momentum based on all the momentum and energy on the RECONSTRUCTED SIDE (signal candidate, p_B_cms is set to 0)
     */
    double ECMissingMass(const Particle* particle, const std::vector<double>& opt);

    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    /**
     * Returns the missing 4-momentum vector in CMS system. Two definitions exist:
     * Option 0: (E)vent based: calculates the missing 4-momentum based on all the momentum and energy in the EVENT
     * Option 1: (C)andidate based: calculates the missing 4-momentum based on all the momentum and energy on the RECONSTRUCTED SIDE (signal candidate, p_B_cms is set to 0)
     */
    TLorentzVector missing4VectorCMS(const Particle* particle, const std::vector<double>& opt);

    /**
     * Returns the neutrino 4-momentum vector. Mass of neutrino is 0 by definition: E = |p|.
     */
    TLorentzVector neutrino4Vector(const Particle* particle);

  }
} // Belle2 namespace

