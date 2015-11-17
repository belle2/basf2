/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Gelb, Thomas Keck                                 *
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
      * return 1 if (dummy)
      */
    double isMajorityInRestOfEventFromB0(const Particle*);

    /**
     * return 1 if (dummy)
     */
    double isMajorityInRestOfEventFromB0bar(const Particle*);

    /**
     * -1 (1) if current RestOfEvent is related to a B0bar (B0) and -2 if empty - used for cuts
     */
    double isRestOfEventEmpty(const Particle* part);

    /**
     * 0 (1) if the RestOfEvent related to the given Particle is related to a B0bar (B0). The MCError of Breco has to be 0 or 1, the output of the variable is -2 otherwise.
     */
    double isRelatedRestOfEventB0Flavor(const Particle* part);

    /**
     * 0 (1) if current RestOfEvent is related to a B0bar (B0). The MCError of Breco has to be 0 or 1, the output of the variable is -2 otherwise.
     */
    double isRestOfEventB0Flavor(const Particle*);

    /**
     * 0 (1) if the majority of tracks and clusters of the RestOfEvent related to the given Particle are related to a B0bar (B0).
     */
    double isRelatedRestOfEventMajorityB0Flavor(const Particle* part);

    /**
     * 0 (1) if the majority of tracks and clusters of the current RestOfEvent are related to a B0bar (B0).
     */
    double isRestOfEventMajorityB0Flavor(const Particle*);

    /**
     * returns missing Momentum on the tag side (flavor tagging specific variable).
     *
     * requires that StoreObjPtr<RestOfEvent> roe("RestOfEvent") exists.
     */
    double p_miss(const Particle*);

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
    double MomentumOfSecondDaughter(const Particle* part);

    /**
     * Returns the Momentum of second daughter if existing in CMS, else 0.
     */
    double MomentumOfSecondDaughter_CMS(const Particle* part);

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
     * requires that StoreObjPtr<ParticleList> KShorts("K_S0:ROELambda") or KShorts("K_S0:ROEKaon") exists.
     */
    Manager::FunctionPtr NumberOfKShortinROEParticleList(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the maximum q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr bestQrOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr InputQrOf(const std::vector<std::string>& arguments);

    /**
     * Returns 1 if the class track by particleName category has the same flavour as the MC target track,
     * 0 else; also if there is no target track. This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr IsRightCategory(const std::vector<std::string>& arguments);

    /**
    * Checks if the given Particle was really a Particle from a B. 1.0 if true otherwise 0.0.
    * This is a Flavor Tagging specific variable!
    */
    Manager::FunctionPtr IsRightTrack(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns Returns 1.0 if the given Particle is classified as target, i.e. if it has the highest probability in particlelistName (argument[0]).
     * The probability is accessed via extraInfoName (argument[1]).
     * This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns Returns the value of the highest target probability in particlelistName (argument[0]).
     * The probability is accessed via extraInfoName (argument[1]).
     * This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr HighestProbInCat(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the semileptonic flavor tagging variables. The Possibilities are:
     * recoilMass: recoiling mass of the Btag system against the target particle in CMS
     * p_missing_CMS: CMS momentum magnitude missing in Btag using as target hypothesis the given particle
     * cosTheta_missing_CMS: cosine of the polar angle of the CMS momentum missing in Btag with respect to the momentum in CMs of the given Particle
     * EW90: Energy in the hemisphere defined by the direction of the virtual W-Boson assuming a semileptonic decay with the given particle as lepton
     * This are Flavor Tagging specific variables!
     */
    Manager::FunctionPtr SemiLeptonicVariables(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the flavor tagging variables corresponding to the FastSlowCorrelated categories.
     * The Possibilities are:
     * p_CMS_Fast: The CMS momentum of the fast track.
     * cosSlowFast: The cosine of the angle between the slow and the fast track.
     * cosTPTO_Fast: The cosine of the angle between the fast track and the thrust axis.
     * SlowFastHaveOpositeCharges: Returns 1 if the fast and slow target particles are oppositely charged, 0 else.
     * This are Flavor Tagging specific variables!
     */
    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments);

    /**
     * Flavour of Btag from trained Method
     */
    double particleClassifiedFlavor(const Particle* particle);

    /**
     * Flavour of Btag from MC
     */
    double particleMCFlavor(const Particle* particle);

    /**
     * return cosine of angle between thrust axis of given particle and thrust axis of ROE
     */
    double cosTPTO(const Particle* part);

  }
}
