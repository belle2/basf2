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
     *
     * TODO: Check this
     */
    double pionVeto(const Particle* particle);

    /**
     * Returns number of remaining neutral ECL clusters in the related RestOfEvent object
     */
    double nROENeutralECLClusters(const Particle* particle);

    /**
     * Returns number of lepton particles in the related RestOfEvent object
     */
    double nROELeptons(const Particle* particle);

    /**
     * Returns total charge of the related RestOfEvent object
     */
    double ROECharge(const Particle* particle);

    /**
     * Returns energy difference of the related RestOfEvent object with respect to E_cms/2
     */
    double ROEDeltaEnergyTag(const Particle* particle);

    /**
     * Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2
     */
    double ROEMassTag(const Particle* particle);

    /**
     * Returns total 4-momentum of the related RestOfEvent object
     */
    TLorentzVector ROE4Vector(const Particle* particle);

  }
} // Belle2 namespace

