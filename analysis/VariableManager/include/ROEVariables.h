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
#include <mdst/dataobjects/MCParticle.h>
#include <TLorentzVector.h>
#include <string>
#include <vector>
#include <set>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Returns 1 if a track, ecl or klmCluster associated to particle is in the related RestOfEvent object, 0 otherwise
     */
    double isInRestOfEvent(const Particle* particle);

    /**
     * Returns number of tracks in the event minus in the current RestOfEvent object
     */
    double nRemainingTracksInRestOfEvent(const Particle* particle);

    /**
     * Returns number of remaining KLM clusters in the related RestOfEvent object
     */
    double nROEKLMClusters(const Particle* particle);

    /**
     * Returns true energy of unused tracks and clusters in ROE.
     */
    double mcROEEnergy(const Particle* particle);

    /**
     * Returns true invariant mass of unused tracks and clusters in ROE
     */
    double mcROEInvariantMass(const Particle* particle);

    /**
     * Returns true momentum of unused tracks and clusters in ROE
     */
    double mcROEMomentum(const Particle* particle);

    /**
     * Returns x component of true momentum of unused tracks and clusters in ROE
     */
    double mcROEMomentumX(const Particle* particle);

    /**
     * Returns y component of true momentum of unused tracks and clusters in ROE
     */
    double mcROEMomentumY(const Particle* particle);

    /**
     * Returns z component of true momentum of unused tracks and clusters in ROE
     */
    double mcROEMomentumZ(const Particle* particle);

    /**
     * Returns flags corresponding to missing particles on ROE side.
     */
    Manager::FunctionPtr ROEMCMissingFlags(const std::vector<std::string>& arguments);

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
     * Returns extra energy in the calorimeter that is not associated to the given Particle
     */
    Manager::FunctionPtr ROEExtraEnergy(const std::vector<std::string>& arguments);

    /**
     * Returns extra energy from neutral ECLClusters in the calorimeter that is not associated to the given Particle
     */
    Manager::FunctionPtr ROENeutralExtraEnergy(const std::vector<std::string>& arguments);

    /**
     * Returns energy of unused tracks and clusters in ROE.
     */
    Manager::FunctionPtr ROEEnergy(const std::vector<std::string>& arguments);

    /**
     * Returns invariant mass of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROEInvariantMass(const std::vector<std::string>& arguments);

    /**
     * Returns momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROEMomentum(const std::vector<std::string>& arguments);

    /**
     * Returns x component of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROEMomentumX(const std::vector<std::string>& arguments);

    /**
     * Returns y component of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROEMomentumY(const std::vector<std::string>& arguments);

    /**
     * Returns z component of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROEMomentumZ(const std::vector<std::string>& arguments);

    /**
     * Returns energy difference of the related RestOfEvent object with respect to E_cms/2 (CMS only)
     */
    Manager::FunctionPtr ROEDeltaE(const std::vector<std::string>& arguments);

    /**
     * Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2 (CMS only)
     */
    Manager::FunctionPtr ROEMbc(const std::vector<std::string>& arguments);

    /**
     * Returns the energy difference of the B meson, corrected with the
     * missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.
     * CMS or LAB (0/1)
     */
    Manager::FunctionPtr correctedBMesonDeltaE(const std::vector<std::string>& arguments);

    /**
     * Returns beam constrained mass of B meson, corrected with the
     * missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.
     * CMS or LAB (0/1)
     */
    Manager::FunctionPtr correctedBMesonMbc(const std::vector<std::string>& arguments);

    /**
     * Returns the invariant mass squared of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missM2(const std::vector<std::string>& arguments);

    /**
     * Returns the polar angle of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missPTheta(const std::vector<std::string>& arguments);

    /**
     * Returns the magnitude of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missP(const std::vector<std::string>& arguments);

    /**
     * Returns the x component of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missPx(const std::vector<std::string>& arguments);

    /**
     * Returns the y component of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missPy(const std::vector<std::string>& arguments);

    /**
     * Returns the z component of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missPz(const std::vector<std::string>& arguments);

    /**
     * Returns the energy of the missing momentum (see possible options)
     */
    Manager::FunctionPtr missE(const std::vector<std::string>& arguments);

    /**
     * Returns Xi_z in event (for Bhabha suppression and two-photon scattering)
     */
    Manager::FunctionPtr xiZ(const std::vector<std::string>& arguments);

    /**
     * Returns the angle between M and lepton in W rest frame in the decays of the type
     * M -> h_1 ... h_n ell, where W 4-momentum is given as pW = p_ell + p_nu. The neutrino
     * momentum is calculated from ROE taking into account the specified mask and setting
     * E_nu = |p_miss|.
     */
    Manager::FunctionPtr cosThetaEll(const std::vector<std::string>& arguments);

    /**
     * Returns boolean value if track or eclCluster type particle passes a certain mask or not. Only to be used in for_each path!
     */
    Manager::FunctionPtr passesROEMask(const std::vector<std::string>& arguments);

    /**
     * Returns custom variable missing mass squared over missing energy
     */
    Manager::FunctionPtr missM2OverMissE(const std::vector<std::string>& arguments);

    /**
     * Returns the momentum transfer squared, q^2, calculated in CMS as q^2 = (p_B - p_h)^2,
     * where p_h is the CMS momentum of all hadrons in the decay B -> H_1 ... H_n ell nu_ell
     * The B meson momentum in CMS is assumed to be 0.
     */
    double q2Bh(const Particle* particle);

    /**
     * Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2,
     * where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter
     */
    Manager::FunctionPtr q2lnu(const std::vector<std::string>& arguments);

    // ------------------------------------------------------------------------------
    // Below are some functions for ease of usage, they are not a part of variables
    // ------------------------------------------------------------------------------

    /**
     * Helper function: Returns the missing 4-momentum vector.
     * Option 0: CMS: Take momentum and energy of all ROE and REC side tracks and clusters into account ("event based" variable)
     * Option 1: CMS: Same as option 0, but fix Emiss = pmiss (missing mass set to 0)
     * Option 2: CMS: Same as option 0, but fix Eroe = Ecms/2 (ignore energy from ROE side)
     * Option 3: CMS: Don't take any ROE tracks and clusters into account, use signal side only (signal based variable)
     * Option 4: CMS: Same as option 3, but use the correction of the B meson momentum magnitude in LAB (update with pB in opposite ROE direction)
     *           system in the opposite direction of the ROE momentum
     * Option 5: LAB: Use momentum and energy of all ROE and REC side tracks and clusters into account ("event based" variable)
     * Option 6: LAB: Same as option 5, but fix Emiss = pmiss (missing mass set to 0)
     * Option 7: LAB: Same as 6, correct pmiss 4vector with factor
     */
    TLorentzVector missing4Vector(const Particle* particle, std::string maskName, std::string opt);

    /**
     * Helper function: Returns bit-pattern of flags corresponding to daughters of MCParticle missing in ROE
     */
    void checkMCParticleMissingFlags(const MCParticle* mcp, std::set<const MCParticle*> mcROEObjects, int& missingFlags);

  }
} // Belle2 namespace

