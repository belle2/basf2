/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Umberto Tamponi               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <framework/gearbox/Const.h>
#include <analysis/VariableManager/Manager.h>

namespace Belle2 {

  namespace Variable {


    /**
     * @return  LogL(particle's hypothesis) for a particle, using an arbitrary combination of sub-detectors
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * pi likelihood using TOP and CDC only =  particleLogLikelihoodValue(211, TOP, CDC);
     * pi likelihood using all the information =  particleLogLikelihoodValue(211, ALL);
     */
    Manager::FunctionPtr pidLogLikelihoodValueExpert(const std::vector<std::string>& arguments);

    /**
     * @return  LogL(hypothesis_1) - LogL(hypothesis_2)  for a particle, using an arbitrary combination of sub-detectors
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * LL(pi) - LL(K) using TOP and CDC only =  deltaLogLikelihoodValue(211, 321, TOP, CDC);
     * LL(pi) - LL(K) using all the information =  particleLogLikelihoodValue(211, 321, ALL);
     */
    Manager::FunctionPtr pidDeltaLogLikelihoodValueExpert(const std::vector<std::string>& arguments);

    /**
     * @return  posterior probability for a certain mass hypothesis  with respect to an alternative hypothesis. Any set of detectors can be used to calculate the likelihood ratios.
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * probability of pi over K, using TOP and CDC only =  particlePairProbability(211, 321, TOP, CDC);
     * probability of K over pi, using ARICH, TOP, CDC only =  particlePairProbability(321, 211, ARICH, TOP, CDC);
     */
    Manager::FunctionPtr pidPairProbabilityExpert(const std::vector<std::string>& arguments);

    /**
    * @return  posterior probability for a certain mass hypothesis, taking into account all the possible alternatives. Any set of detectors can be used to calculate the likelihood ratios.
    * For expert's use only!!
    * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
    * Examples:
    * probability of pi hypothesis, using TOP and CDC only =  particleProbability(211, TOP, CDC);
    * probability of K hypothesis, using ARICH, TOP, CDC only =  particleProbability(321, ARICH, TOP, CDC);
    */
    Manager::FunctionPtr pidParticleProbabilityExpert(const std::vector<std::string>& arguments);

    /**
    * @return  returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
    */
    Manager::FunctionPtr pidMissingProbabilityExpert(const std::vector<std::string>& arguments);


    /**
     * return electron ID to be used in the physics analyses
     */
    double electronID(const Particle* part);

    /**
     * return muon ID to be used in the physics analyses
     */
    double muonID(const Particle* part);

    /**
     * return pion ID to be used in the physics analyses
     */
    double pionID(const Particle* part);

    /**
     * return kaon ID to be used in the physics analyses
     */
    double kaonID(const Particle* part);

    /**
     * return proton ID to be used in the physics analyses
     */
    double protonID(const Particle* part);

    /**
     * return deuteron ID to be used in the physics analyses
     */
    double deuteronID(const Particle* part);

    /**
     * Returns Belle's main PID variable to separate pions, kaons and protons:  atc_pid(3,1,5).prob()
     * Additional arguments are intigers for signal and background hypothesis:
     * (0 = electron, 1 = muon, 2 = pion, 3 = kaon, 4 = proton)
     * This variable should only be used to analyse converted Belle samples. Do not use to analyse Belle II samples.
     */
    double atcPIDBelle(const Particle*, const std::vector<double>& sigAndBkgHyp);

    /**
     * Returns Belle's muonID variable.
     **/
    double muIDBelle(const Particle*);

    /**
     * Returns Belle's muonID quality variable.
     **/
    double muIDBelleQuality(const Particle*);

    /**
     * Returns Belle's eID variable.
     **/
    double eIDBelle(const Particle*);

    /**
     * Returns Belle's kaonID.
     **/
    double kIDBelle(const Particle*);



  }
} // Belle2 namespace

