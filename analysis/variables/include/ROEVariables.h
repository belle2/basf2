/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <analysis/VariableManager/Manager.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <TLorentzVector.h>
#include <string>
#include <vector>
#include <set>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Returns 1 if a track, ecl or klmCluster associated to particle is in the related RestOfEvent object, 0 otherwise.
     * Also works for composite particles, where all mdst objects of related FSP particles must be in ROE.
     */
    double isInRestOfEvent(const Particle* particle);
    /**
     * Returns 1 if a particle is a clone of signal side final state particles, 0 otherwise.
     */
    double isCloneOfSignalSide(const Particle* particle);

    /**
     * Returns 1 if a particle has ancestor signal side final state particles, 0 otherwise.
     */
    double hasAncestorFromSignalSide(const Particle* particle);

    /**
     * Prints the indices of all particles in the ROE and the properties of all masks appended to the ROE.
     * Intended for debugging purposes, always returns 0.
     */
    double printROE(const Particle* particle);

    /**
     * Helper function for nRemainingTracksInRestOfEventWithMask and nRemainingTracksInRestOfEvent
     */
    double nRemainingTracksInROE(const Particle* particle, const std::string& maskName = RestOfEvent::c_defaultMaskName);

    /**
     * Returns number of tracks in the event minus in the current RestOfEvent object accepting a mask.
     */
    Manager::FunctionPtr nROE_RemainingTracksWithMask(const std::vector<std::string>& arguments);

    /**
     * Returns variable in Rest of Event rest frame.
     */
    Manager::FunctionPtr useROERecoilFrame(const std::vector<std::string>& arguments);

    /**
     * Returns number of tracks in the event minus in the current RestOfEvent object
     */
    double nROE_RemainingTracks(const Particle* particle);

    /**
     * Returns number of remaining KLM clusters in the related RestOfEvent object
     */
    double nROE_KLMClusters(const Particle* particle);

    /**
     * Returns true energy of unused tracks and clusters in ROE.
     */
    double ROE_MC_E(const Particle* particle);

    /**
     * Returns true invariant mass of unused tracks and clusters in ROE
     */
    double ROE_MC_M(const Particle* particle);

    /**
     * Returns true momentum of unused tracks and clusters in ROE
     */
    double ROE_MC_P(const Particle* particle);

    /**
     * Returns x component of true momentum of unused tracks and clusters in ROE
     */
    double ROE_MC_Px(const Particle* particle);

    /**
     * Returns y component of true momentum of unused tracks and clusters in ROE
     */
    double ROE_MC_Py(const Particle* particle);

    /**
     * Returns z component of true momentum of unused tracks and clusters in ROE
     */
    double ROE_MC_Pz(const Particle* particle);

    /**
     * Returns true transverse momentum of unused tracks and clusters in ROE
     */
    double ROE_MC_Pt(const Particle* particle);

    /**
     * Returns true polar angle of momentum of unused tracks and clusters in ROE
     */
    double ROE_MC_PTheta(const Particle* particle);

    /**
     * Returns flags corresponding to missing particles on ROE side.
     */
    Manager::FunctionPtr ROE_MC_MissingFlags(const std::vector<std::string>& arguments);

    /**
     * Returns number of tracks in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_Tracks(const std::vector<std::string>& arguments);

    /**
     * Returns number of ECL clusters in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_ECLClusters(const std::vector<std::string>& arguments);

    /**
     * Returns number of neutral ECL clusters in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_NeutralECLClusters(const std::vector<std::string>& arguments);

    /**
     * Returns number of charged particles in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_ChargedParticles(const std::vector<std::string>& arguments);

    /**
     * Returns number of photons in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_Photons(const std::vector<std::string>& arguments);

    /**
     * Returns number of neutral hadrons in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_NeutralHadrons(const std::vector<std::string>& arguments);

    /**
     * Returns number of V0s in the related RestOfEvent object that pass the selection criteria
     */
    Manager::FunctionPtr nROE_Composites(const std::vector<std::string>& arguments);

    /**
     * Returns the number of particles in ROE from the given particle list.
     * Use of variable aliases is advised.
     */
    Manager::FunctionPtr nROE_ParticlesInList(const std::vector<std::string>& arguments);

    /**
     * Returns total charge of the related RestOfEvent object
     */
    Manager::FunctionPtr ROE_Charge(const std::vector<std::string>& arguments);

    /**
     * Returns extra energy in the calorimeter that is not associated to the given Particle
     */
    Manager::FunctionPtr ROE_ExtraEnergy(const std::vector<std::string>& arguments);

    /**
     * Returns extra energy from neutral ECLClusters in the calorimeter that is not associated to the given Particle
     */
    Manager::FunctionPtr ROE_NeutralExtraEnergy(const std::vector<std::string>& arguments);

    /**
     * Returns energy of unused tracks and clusters in ROE.
     */
    Manager::FunctionPtr ROE_E(const std::vector<std::string>& arguments);

    /**
     * Returns invariant mass of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_M(const std::vector<std::string>& arguments);

    /**
     * Returns momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_P(const std::vector<std::string>& arguments);

    /**
     * Returns x component of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_Px(const std::vector<std::string>& arguments);

    /**
     * Returns y component of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_Py(const std::vector<std::string>& arguments);

    /**
     * Returns z component of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_Pz(const std::vector<std::string>& arguments);

    /**
     * Returns transverse momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_Pt(const std::vector<std::string>& arguments);

    /**
     * Returns theta angle of momentum of unused tracks and clusters in ROE
     */
    Manager::FunctionPtr ROE_PTheta(const std::vector<std::string>& arguments);

    /**
     * Returns energy difference of the related RestOfEvent object with respect to E_cms/2 (CMS only)
     */
    Manager::FunctionPtr ROE_DeltaE(const std::vector<std::string>& arguments);

    /**
     * Returns beam constrained mass of the related RestOfEvent object with respect to E_cms/2 (CMS only)
     */
    Manager::FunctionPtr ROE_Mbc(const std::vector<std::string>& arguments);

    /**
     * Returns the energy difference of the B meson, corrected with the
     * missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.
     * CMS or LAB (0/1). "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_DeltaE(const std::vector<std::string>& arguments);

    /**
     * Returns beam constrained mass of B meson, corrected with the
     * missing neutrino momentum (reconstructed side + neutrino) with respect to E_cms/2.
     * CMS or LAB (0/1)."WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_Mbc(const std::vector<std::string>& arguments);

    /**
     * Returns the invariant mass squared of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissM2(const std::vector<std::string>& arguments);

    /**
     * Returns the polar angle of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissPTheta(const std::vector<std::string>& arguments);

    /**
     * Returns the magnitude of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissP(const std::vector<std::string>& arguments);

    /**
     * Returns the x component of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissPx(const std::vector<std::string>& arguments);

    /**
     * Returns the y component of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissPy(const std::vector<std::string>& arguments);

    /**
     * Returns the z component of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissPz(const std::vector<std::string>& arguments);

    /**
     * Returns the energy of the missing momentum.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissE(const std::vector<std::string>& arguments);

    /**
     * Returns Xi_z in event (for Bhabha suppression and two-photon scattering)
     */
    Manager::FunctionPtr ROE_xiZ(const std::vector<std::string>& arguments);

    /**
     * Returns the angle between M and lepton in W rest frame in the decays of the type
     * M -> h_1 ... h_n ell, where W 4-momentum is given as pW = p_ell + p_nu. The neutrino
     * momentum is calculated from ROE taking into account the specified mask and setting
     * E_nu = |p_miss|."WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_cosThetaEll(const std::vector<std::string>& arguments);

    /**
     * Returns boolean value if track or eclCluster type particle passes a certain mask or not. Only to be used in for_each path!
     */
    Manager::FunctionPtr passesROEMask(const std::vector<std::string>& arguments);

    /**
     * Returns custom variable missing mass squared over missing energy.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_MissM2OverMissE(const std::vector<std::string>& arguments);

    /**
     * Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2,
     * where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_q2lnuSimple(const std::vector<std::string>& arguments);

    /**
     * Returns the momentum transfer squared, q^2, calculated in LAB as q^2 = (p_l + p_nu)^2,
     * where B -> H_1 ... H_n ell nu_ell. Lepton is assumed to be the last reconstructed daughter.
     * This calculation uses constraints from dE = 0 and Mbc = Mb to correct the neutrino direction.
     * "WE" was to do with the "direction" of the W boson in semileptonic decays (the lepton-neutrino system).
     */
    Manager::FunctionPtr WE_q2lnu(const std::vector<std::string>& arguments);

    /**
     * Returns pi0 probability, where mode is used to specify the selection criteria for soft photon.
     * The following strings are available.
     * standard: loose energy cut and no clusterNHits cut are applied to soft photon
     * tight: tight energy cut and no clusterNHits cut are applied to soft photon
     * cluster: loose energy cut and clusterNHits cut are applied to soft photon
     * both: tight energy cut and clusterNHits cut are applied to soft photon
     * You can find more details in writePi0EtaVeto function in modularAnalysis.py.
     */
    Manager::FunctionPtr pi0Prob(const std::vector<std::string>& arguments);

    /**
     * Returns eta probability, where mode is used to specify the selection criteria for soft photon.
     * The following strings are available.
     * standard: loose energy cut and no clusterNHits cut are applied to soft photon
     * tight: tight energy cut and no clusterNHits cut are applied to soft photon
     * cluster: loose energy cut and clusterNHits cut are applied to soft photon
     * both: tight energy cut and clusterNHits cut are applied to soft photon
     * You can find more details in writePi0EtaVeto function in modularAnalysis.py.
     */
    Manager::FunctionPtr etaProb(const std::vector<std::string>& arguments);


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
    TLorentzVector missing4Vector(const Particle* particle, const std::string& maskName, const std::string& opt);

    /**
     * Helper function: Returns bit-pattern of flags corresponding to daughters of MCParticle missing in ROE
     */
    void checkMCParticleMissingFlags(const MCParticle* mcp, std::set<const MCParticle*> ROE_MCObjects, int& missingFlags);

    /**
     * Helper function: Returns 1 if a track, ecl or klmCluster associated to the particle is in the related RestOfEvent object, 0 otherwise.
     * Also works for composite particles, where all mdst objects of related FSP particles must be in ROE.
     * This helper function accepts a specific roe object as an argument
     */
    double isInThisRestOfEvent(const Particle* particle, const RestOfEvent* roe,
                               const std::string& maskName = RestOfEvent::c_defaultMaskName);


    /**
     * temp
     */
    Manager::FunctionPtr bssMassDifference(const std::vector<std::string>& arguments);
    /**
     * returns related nested or host ROE
     */
    const RestOfEvent* getRelatedROEObject(const Particle* particle, bool returnHostOnly = false);
  }
} // Belle2 namespace

