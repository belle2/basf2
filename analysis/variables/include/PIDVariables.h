/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {


    /**
     * @return  LogL(particle's hypothesis) for a particle, using an arbitrary combination of sub-detectors
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * pi likelihood using TOP and CDC only = pidLogLikelihoodValueExpert(211, TOP, CDC);
     * pi likelihood using all the information = pidLogLikelihoodValueExpert(211, ALL);
     */
    Manager::FunctionPtr pidLogLikelihoodValueExpert(const std::vector<std::string>& arguments);

    /**
     * @return  LogL(hypothesis_1) - LogL(hypothesis_2)  for a particle, using an arbitrary combination of sub-detectors
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * LL(pi) - LL(K) using TOP and CDC only = pidDeltaLogLikelihoodValueExpert(211, 321, TOP, CDC);
     * LL(pi) - LL(K) using all the information = pidDeltaLogLikelihoodValueExpert(211, 321, ALL);
     */
    Manager::FunctionPtr pidDeltaLogLikelihoodValueExpert(const std::vector<std::string>& arguments);

    /**
     * @return  posterior probability for a certain mass hypothesis  with respect to an alternative hypothesis. Any set of detectors can be used to calculate the likelihood ratios.
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * probability of pi over K, using TOP and CDC only = pidPairProbabilityExpert(211, 321, TOP, CDC);
     * probability of K over pi, using ARICH, TOP, CDC only = pidPairProbabilityExpert(321, 211, ARICH, TOP, CDC);
     */
    Manager::FunctionPtr pidPairProbabilityExpert(const std::vector<std::string>& arguments);

    /**
    * @return  posterior probability for a certain mass hypothesis, taking into account all the possible alternatives. Any set of detectors can be used to calculate the likelihood ratios.
    * For expert's use only!!
    * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
    * Examples:
    * probability of pi hypothesis, using TOP and CDC only = pidProbabilityExpert(211, TOP, CDC);
    * probability of K hypothesis, using ARICH, TOP, CDC only = pidProbabilityExpert(321, ARICH, TOP, CDC);
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
     * SPECIAL (TEMP) variable (BII-8444)
     * @return electron ID without TOP information
     */
    double electronID_noTOP(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8444).
     * @return binary PID between electron hypothesis and another hypothesis, without TOP information.
     */
    double binaryElectronID_noTOP(const Particle* part, const std::vector<double>& arguments);

    /**
     * SPECIAL (TEMP) variable (BII-8444, BII-8760).
     * @return electron ID without SVD and TOP information.
     */
    double electronID_noSVD_noTOP(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-8444, BII-8760).
     * @return binary PID between electron hypothesis and another hypothesis, without TOP information.
     */
    double binaryElectronID_noSVD_noTOP(const Particle* part, const std::vector<double>& arguments);

    /**
     * SPECIAL (TEMP) variable (BII-9461)
     * @return pion ID with special ARICH likelihood treatment
     */
    double pionID_noARICHwoECL(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-9461)
     * @return kaon ID with special ARICH likelihood treatment
     */
    double kaonID_noARICHwoECL(const Particle* part);

    /**
     * SPECIAL (TEMP) variable (BII-9461)
     * @return binary PID between two particle hypotheses with special ARICH likelihood treatment
     */
    double binaryPID_noARICHwoECL(const Particle* part, const std::vector<double>& arguments);

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
    double mostLikelyPDG(const Particle* part, const std::vector<double>& arguments);

    /**
     * returns true if a particle is assigned to its most likely type according to PID likelihood
     */
    bool isMostLikely(const Particle* part, const std::vector<double>& arguments);

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
     * @return  Weighted LogL(particle's hypothesis) for a particle, using an arbitrary combination of sub-detectors
     * For expert's use only!!
     * The first argument should be the db object name of the calibration weight matrix.
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * pi likelihood using TOP and CDC only =  pidWeightedLogLikelihoodValueExpert(CalibrationWeightMatrix, 211, TOP, CDC);
     * pi likelihood using all the information =  pidWeightedLogLikelihoodValueExpert(CalibrationWeightMatrix,211, ALL);
     */
    Manager::FunctionPtr pidWeightedLogLikelihoodValueExpert(const std::vector<std::string>& arguments);

    /**
     * @return  weighted posterior probability for a certain mass hypothesis  with respect to an alternative hypothesis. Any set of detectors can be used to calculate the likelihood ratios.
     * For expert's use only!!
     * The first argument should be the db object name of the calibration weight matrix.
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
     * Examples:
     * probability of pi over K, using TOP and CDC only =  pidWeightedPairProbabilityExpert(CalibrationWeightMatrix, 211, 321, TOP, CDC);
     * probability of K over pi, using ARICH, TOP, CDC only =  pidWeightedPairProbabilityExpert(CalibrationWeightMatrix, 321, 211, ARICH, TOP, CDC);
     */
    Manager::FunctionPtr pidWeightedPairProbabilityExpert(const std::vector<std::string>& arguments);

    /**
     * @return neural-network based PID likelihood
     * For expert's use only!!
     * The first argument is the particle hypothesis as string
     * The second argument is the db object name of the neural network parameters (optional).
     * Examples:
     * pi probability =  pidNeuralNetworkValueExpert(211, PIDNeuralNetworkParameters);
     */
    Manager::FunctionPtr pidNeuralNetworkValueExpert(const std::vector<std::string>& arguments);

    /**
    * @return  weighted posterior probability for a certain mass hypothesis, taking into account all the possible alternatives. Any set of detectors can be used to calculate the likelihood ratios.
    * For expert's use only!!
    * The first argument should be the db object name of the calibration weight matrix.
    * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM} or ALL.
    * Examples:
    * probability of pi hypothesis, using TOP and CDC only =  pidWeightedProbabilityExpert(CalibrationWeightMatrix, 211, TOP, CDC);
    * probability of K hypothesis, using ARICH, TOP, CDC only =  pidWeightedProbabilityExpert(CalibrationWeightMatrix, 321, ARICH, TOP, CDC);
    */
    Manager::FunctionPtr pidWeightedProbabilityExpert(const std::vector<std::string>& arguments);


    /**
     * @return weighted electron ID to be used in the physics analyses
     */
    Manager::FunctionPtr weightedElectronID(const std::vector<std::string>& arguments);

    /**
     * @return weighted muon ID to be used in the physics analyses
     */
    Manager::FunctionPtr weightedMuonID(const std::vector<std::string>& arguments);

    /**
     * @return weighted pion ID to be used in the physics analyses
     */
    Manager::FunctionPtr weightedPionID(const std::vector<std::string>& arguments);

    /**
     * @return weighted kaon ID to be used in the physics analyses
     */
    Manager::FunctionPtr weightedKaonID(const std::vector<std::string>& arguments);

    /**
     * @return weighted proton ID to be used in the physics analyses
     */
    Manager::FunctionPtr weightedProtonID(const std::vector<std::string>& arguments);

    /**
     * @return weighted deuteron ID to be used in the physics analyses
     */
    Manager::FunctionPtr weightedDeuteronID(const std::vector<std::string>& arguments);


    /**
     * @return electron ID from Neural Network to be used in the physics analyses
     */
    double electronIDNN(const Particle* particle);

    /**
     * @return muon ID from Neural Network to be used in the physics analyses
     */
    double muonIDNN(const Particle* particle);

    /**
     * @return pion ID from Neural Network to be used in the physics analyses
     */
    double pionIDNN(const Particle* particle);

    /**
     * @return kaon ID from Neural Network to be used in the physics analyses
     */
    double kaonIDNN(const Particle* particle);

    /**
     * @return proton ID from Neural Network to be used in the physics analyses
     */
    double protonIDNN(const Particle* particle);

    /**
     * @return deuteron ID from Neural Network to be used in the physics analyses
     */
    double deuteronIDNN(const Particle* particle);

    /**
     * Parses the detector list for the PID metafunctions.
     **/
    Const::PIDDetectorSet parseDetectors(const std::vector<std::string>& arguments);

  }
} // Belle2 namespace
