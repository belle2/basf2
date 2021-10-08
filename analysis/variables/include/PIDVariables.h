/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
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
    Manager::FunctionPtr pidProbabilityExpert(const std::vector<std::string>& arguments);

    /**
    * @return  returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
    */
    Manager::FunctionPtr pidMissingProbabilityExpert(const std::vector<std::string>& arguments);

    /**
     * @return The particle ID (of the particle's own hypothesis)
     */
    double particleID(const Particle* part);

    /**
     * @return electron ID to be used in the physics analyses
     */
    double electronID(const Particle* part);

    /**
     * @return muon ID to be used in the physics analyses
     */
    double muonID(const Particle* part);

    /**
     * @return pion ID to be used in the physics analyses
     */
    double pionID(const Particle* part);

    /**
     * @return kaon ID to be used in the physics analyses
     */
    double kaonID(const Particle* part);

    /**
     * @return proton ID to be used in the physics analyses
     */
    double protonID(const Particle* part);

    /**
     * @return deuteron ID to be used in the physics analyses
     */
    double deuteronID(const Particle* part);

    /**
     * @return binary PID between two particle hypotheses
     */
    double binaryPID(const Particle* part, const std::vector<double>& arguments);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return electron ID without SVD information
     */
    double electronID_noSVD(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return muon ID without SVD information
     */
    double muonID_noSVD(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return pion ID without SVD information
     */
    double pionID_noSVD(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return kaon ID without SVD information
     */
    double kaonID_noSVD(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return proton ID without SVD information
     */
    double protonID_noSVD(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return deuteron ID without SVD information
     */
    double deuteronID_noSVD(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8760)
     * @return binary PID between two particle hypotheses without SVD information
     */
    double binaryPID_noSVD(const Particle* part, const std::vector<double>& arguments);

    /**
    * returns the MVA score for anti-neutron PID (not for neutron)
    * -1 means invalid
    *  0 background-like
    *  1 signal-like
    */
    double antineutronID(const Particle* particle);

    /**
     * @return the charged PID BDT score for a certain mass hypothesis with respect to all other charged stable particle hypotheses.
     *
     * The signal hypothesis pdgId and the detector(s) used for the BDT training are passed as a vector of strings of size = 2.
     * Examples:
     * Response of BDT trained for multi-class separation, "e vs. others", BDT training based on all sub-detectors = pidChargedBDTScore(11, ALL)
     *
     * If the response for the given hypothesis and detector(s) is not available for the particle under test, return NaN.
     */
    Manager::FunctionPtr pidChargedBDTScore(const std::vector<std::string>& pdgCodeHyp);

    /**
     * @return the charged PID BDT score for a certain mass hypothesis with respect to an alternative hypothesis.
     *
     * The signal hypothesis pdgId, the test pdgId and the detector(s) used for the BDT training are passed as a vector of strings of size = 3.
     * Examples:
     * Response of BDT trained for binary "e vs pi" separation, BDT training based on all sub-detectors = pidPairChargedBDTScore(11, 211, ALL)
     *
     * If the response for the given set of hypotheses is not available for the particle under test, return NaN.
     */
    Manager::FunctionPtr pidPairChargedBDTScore(const std::vector<std::string>& arguments);

    /**
     * returns most likely PDG code based on PID information.
     */
    Manager::FunctionPtr mostLikelyPDG(const std::vector<std::string>& arguments);

    /**
     * returns true if a particle is assigned to its most likely type according to PID likelihood
     */
    Manager::FunctionPtr isMostLikely(const std::vector<std::string>& arguments);

    /**
     * Returns Belle's main PID variable to separate pions, kaons and protons:  atc_pid(3,1,5).prob()
     * Additional arguments are integers for signal and background hypothesis:
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
     * Parses the detector list for the PID metafunctions.
     **/
    Const::PIDDetectorSet parseDetectors(const std::vector<std::string>& arguments);

  }
} // Belle2 namespace
