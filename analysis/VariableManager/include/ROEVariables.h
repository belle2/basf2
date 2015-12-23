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
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <TLorentzVector.h>
#include <map>
#include <string>
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Returns 1 if a track, ecl or klmCluster associated to particle is in the related RestOfEvent object, 0 otherwise
     */
    double isInRestOfEvent(const Particle* particle);

    /**TODO:
     * Returns number of tracks in the event minus in the current RestOfEvent object
     */
    double nRemainingTracksInRestOfEvent(const Particle* particle);

    /**TODO:
     * Returns 1 if the invariant mass of a combination of a photon in RestOfEvent with
     * the signal photon yields the mass of the a neutral Pion.
     */
    double pionVeto(const Particle* particle);

    /**
     * Returns number of all the tracks in the related RestOfEvent object
     */
    double nAllROETracks(const Particle* particle);

    /**
     * Returns number of all ECL clusters in the related RestOfEvent object
     */
    double nAllROEECLClusters(const Particle* particle);

    /**
     * Returns number of all neutral ECL clusters in the related RestOfEvent object
     */
    double nAllROENeutralECLClusters(const Particle* particle);

    /**
     * Returns number of remaining KLM clusters in the related RestOfEvent object
     */
    double nAllROEKLMClusters(const Particle* particle);

    /**
     * Returns MC Errors for an artificial tag side particle, which corresponds to the ROE object
     */
    double ROEMCErrors(const Particle* particle);

    /**
     * Returns number of tracks in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROETracks(const std::vector<std::string>& arguments);

    /**
     * Returns number of ECL clusters in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROEECLClusters(const std::vector<std::string>& arguments);

    /**
     * Returns number of neutral ECL clusters in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROENeutralECLClusters(const std::vector<std::string>& arguments);

    /**
     * Returns number of neutral pions, constructed from good gamma candidates in the related RestOfEvent object that passed the selection criteria
     */
    Manager::FunctionPtr nROEPi0s(const std::vector<std::string>& arguments);

    /**
     * Returns total charge of the related RestOfEvent object
     */
    Manager::FunctionPtr ROECharge(const std::vector<std::string>& arguments);

    /**
     * return extra energy in the calorimeter that is not associated to the given Particle
     */
    Manager::FunctionPtr ROEExtraEnergy(const std::vector<std::string>& arguments);

    /**
     * Returns energy difference of the related RestOfEvent object with respect to E_cms/2
     */
    Manager::FunctionPtr ROEDeltaE(const std::vector<std::string>& arguments);

    /**
     * Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2
     */
    Manager::FunctionPtr ROEMbc(const std::vector<std::string>& arguments);

    /**
     * Returns the energy difference of the B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2
     */
    Manager::FunctionPtr correctedBMesonDeltaE(const std::vector<std::string>& arguments);

    /**
     * Returns beam constrained mass of B meson, corrected with the missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2
     */
    Manager::FunctionPtr correctedBMesonMbc(const std::vector<std::string>& arguments);

    /**
     * Returns the missing mass squared.
     * Option 0: Take momentum and energy of all ROE tracks and clusters into account
     * Option 1: Take only momentum of ROE tracks and clusters into account, energy set to E_beam
     * Option 2: Don't take any ROE tracks and clusters into account, use signal side only
     * Option 3: Same as option 2, but use the correction of the B meson momentum magnitude in LAB
     *           system in the direction of the ROE momentum
     */
    Manager::FunctionPtr ROEMissingMass(const std::vector<std::string>& arguments);

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
    TLorentzVector missing4VectorCMS(const Particle* particle, std::string maskName, std::string opt);

    /**
     * Returns the neutrino 4-momentum vector in CMS system. Mass of neutrino is 0 by definition: E == |p|
     */
    TLorentzVector neutrino4VectorCMS(const Particle* particle, std::string maskName);

  }
} // Belle2 namespace

