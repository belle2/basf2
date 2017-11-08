/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Fernando Abudinen, Moritz Gelb, Thomas Keck              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns missing Momentum on the tag side (flavor tagging specific variable).
     *
     * requires that StoreObjPtr<RestOfEvent> roe("RestOfEvent") exists.
     */
    double momentumMissingTagSide(const Particle*);

    /**
     * return cosine of angle between thrust axis of given particle and thrust axis of ROE
     */
    double cosTPTO(const Particle* part);

    /**
     * 1.0 if pdg-code for Lambda0, -1.0 if Anti-Lambda0, 0.0 else
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double lambdaFlavor(const Particle* particle);

    /**
     * 0.0 if pdg-code for MCParticle is Lambda0, 0.0 if MCAnti-Lambda0, 1.0 else
     *
     */
    double isLambda(const Particle* particle);

    /**
     * Returns the Matrixelement[2][2] of the PositionErrorMatrix of the Vertex fit.
     *
     * This is a simplistic hack. But I see no other way to get the information.
     */
    double lambdaZError(const Particle* particle);

    /**
     * Returns the Momentum of second daughter if existing, else 0.
     */
    double momentumOfSecondDaughter(const Particle* part);

    /**
     * Returns the Momentum of second daughter if existing in CMS, else 0.
     */
    double momentumOfSecondDaughterCMS(const Particle* part);

    /**
     * Returns q*(highest PID_Likelihood for Kaons), else 0.
     */
    double chargeTimesKaonLiklihood(const Particle*);

    /**
     * Returns the transverse momentum of all charged tracks if there exists a ROE for the given particle, else 0.
     */
    double transverseMomentumOfChargeTracksInRoe(const Particle* part);

    /**
     * Returns the number of K_S0 in the given ROE Particle List(flavor tagging specific variable).
     *
     * requires that StoreObjPtr<ParticleList> KShorts("K_S0:inRoe") exists.
     */
    double NumberOfKShortsInRoe(const Particle* particle);

    /**
     * returns 1.0 if the particle has been selected as target in the muon or electron flavor tagging category, 0.0 else.
     */
    double isInElectronOrMuonCat(const Particle* particle);

    /**
      * return 1 if (dummy)
      */
    double isMajorityInRestOfEventFromB0(const Particle*);

    /**
     * return 1 if (dummy)
     */
    double isMajorityInRestOfEventFromB0bar(const Particle*);

    /**
     * -1 (1) if current RestOfEvent is related to a B0bar (B0) has tracks. Used for cuts
     */
    double hasRestOfEventTracks(const Particle* part);

    /**
     * -1 (1) if the RestOfEvent related to the given Particle is related to a B0bar (B0). The MCError bit of Breco has to be 0, 1, 2, 16 or 1024. The output of the variable is 0 otherwise. If one Particle in the Rest of Event is found to belong the reconstructed B0, the output is -2(2) for a B0bar (B0) on the CP side.
     */
    double isRelatedRestOfEventB0Flavor(const Particle* part);

    /**
     * -1 (1) if current RestOfEvent is related to a B0bar (B0). The MCError bit of Breco has to be 0, 1, 2, 16 or 1024. The output of the variable is 0 otherwise. If one Particle in the Rest of Event is found to belong the reconstructed B0, the output is -2(2) for a B0bar (B0) on the CP side.
     */
    double isRestOfEventB0Flavor(const Particle*);

    /**
     * checks the decay chain upwards up to the Y(4S) resonance.Output is 0 (1) if an ancestor is found to be a B0bar (B0), if not -2.
     */
    double ancestorHasWhichFlavor(const Particle* particle);

    /**
     * mcErrors MCMatching Flag on the reconstructed B0_cp.
     */
    double B0mcErrors(const Particle* particle);

    /**
     * 0 (1) if the majority of tracks and clusters of the RestOfEvent related to the given Particle are related to a B0bar (B0).
     */
    double isRelatedRestOfEventMajorityB0Flavor(const Particle* part);

    /**
     * 0 (1) if the majority of tracks and clusters of the current RestOfEvent are related to a B0bar (B0).
     */
    double isRestOfEventMajorityB0Flavor(const Particle*);

    /**
     * returns the PDG code of the correspondig B0/B0bar when found in RoE, 0 otherwise
     */
    double McFlavorOfTagSide(const Particle* part);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the semileptonic flavor tagging variables. The Possibilities are:
     * recoilMass: recoiling mass of the Btag system against the target particle in CMS
     * pMissCMS: CMS momentum magnitude missing in Btag using as target hypothesis the given particle
     * cosThetaMissCMS: cosine of the polar angle of the CMS momentum missing in Btag with respect to the momentum in CMS of the given Particle
     * EW90: Energy in the hemisphere defined by the direction of the virtual W-Boson assuming a semileptonic decay with the given particle as lepton
     * This are Flavor Tagging specific variables!
     */
    Manager::FunctionPtr BtagToWBosonVariables(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the flavor tagging variables corresponding to the KaonPion category.
     * The Possibilities are:
     * HaveOpositeCharges: 1 if the particles selected as target kaon and slow pion in the respective flavour tagging categories have oposite charges, 0 else.
     * cosKaonPion: Cosine of angle between kaon and slow pion momenta, i.e. between the momenta of the particles selected as target kaon and slow pion.
     */
    Manager::FunctionPtr KaonPionVariables(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the flavor tagging variables corresponding to the FastSlowCorrelated category.
     * The Possibilities are:
     * pFastCMS: The CMS momentum of the fast track.
     * cosSlowFast: The cosine of the angle between the slow and the fast track.
     * cosTPTOFast: The cosine of the angle between the fast track and the thrust axis.
     * SlowFastHaveOpositeCharges: Returns 1 if the fast and slow target particles are oppositely charged, 0 else.
     * This are Flavor Tagging specific variables!
     */
    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1.0 if the given Particle is classified as target, i.e. if it has the highest probability in particlelistName (argument[0]).
     * The probability is accessed via extraInfoName (argument[1]).
     * This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the value of the highest target probability in particlelistName (argument[0]).
     * The probability is accessed via extraInfoName (argument[1]).
     * This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr HighestProbInCat(const std::vector<std::string>& arguments);

    /**
    * Checks if the given Particle was really a Particle from a B. 1.0 if true otherwise 0.0.
    * This is a Flavor Tagging specific variable!
    */
    Manager::FunctionPtr isRightTrack(const std::vector<std::string>& arguments);

    /**
     * Returns 1 if the class track by particleName category has the same flavour as the MC target track,
     * 0 else; also if there is no target track. This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr isRightCategory(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr weightedQrOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1.0 if the given category has at least one target.
     * This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr hasTrueTarget(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1.0 if the given category tags the B0 MC flavor correctly.
     * This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr isTrueCategory(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the output of the flavorTagger for the given combinerMethod.
     * This is a Flavor Tagging variable for general use.
     */
    Manager::FunctionPtr qrOutput(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the flavor tag q output of the flavorTagger for the given combinerMethod.
     * This is a Flavor Tagging variable for general use.
     */
    Manager::FunctionPtr qOutput(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the corresponding r (dilution) bin according to the Belle binning for the given combinerMethod.
     * The default methods are 'FBDT' or 'FANN'.
     * This is a Flavor Tagging variable for general use.
     */
    Manager::FunctionPtr rBinBelle(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the output q (charge of target track) times p (probability that this is the right category) of the category
     * with the given name. The allowed categories are the official Flavor Tagger Category Names.
     * This is a Flavor Tagging variable for general use.
     */
    Manager::FunctionPtr qpCategory(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if the target particle (checking the decay chain) of the category with the given name is found in the mc Particles,
     * and if it provides the right Flavor. The allowed categories are the official Flavor Tagger Category Names.
     * This is a Flavor Tagging variable for general use.
     */
    Manager::FunctionPtr isTrueFTCategory(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if target particles (checking only the decay chain) of the category with the given name
     * is found in the mc Particles. The allowed categories are the official Flavor Tagger Category Names.
     * This is a Flavor Tagging variable for general use.
     */
    Manager::FunctionPtr hasTrueTargets(const std::vector<std::string>& arguments);


  }
}
