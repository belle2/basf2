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
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM},  ALL, or DEFAULT (=SVD+CDC+TOP+ARICH).
     * Exmaples:
     * pi likelihood using TOP and CDC only =  particleLogLikelihoodValue(211, TOP, CDC);
     * pi likelihood using all the information =  particleLogLikelihoodValue(211, ALL);
     */
    Manager::FunctionPtr particleLogLikelihoodValue(const std::vector<std::string>& arguments);

    /**
     * @return  posterior probability for a certain mass hypothesis  with respect to an alternative hypothesis. Any set of detectors can be used to calculate the likelihood ratios.
     * For expert's use only!!
     * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
     * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM},  ALL, or DEFAULT (=SVD+CDC+TOP+ARICH).
     * Exmaples:
     * probability of pi over K, using TOP and CDC only =  particlePairProbability(211, 321, TOP, CDC);
     * probability of K over pi, using ARICH, TOP, CDC only =  particlePairProbability(321, 211, ARICH, TOP, CDC);
     */
    Manager::FunctionPtr particlePairProbability(const std::vector<std::string>& arguments);

    /**
    * @return  posterior probability for a certain mass hypothesis, taking into account all the possible alternatives. Any set of detectors can be used to calculate the likelihood ratios.
    * For expert's use only!!
    * The particle hypothesis and the detectors combination to be used for the likelihood calculation are passed as a vector of strings.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM},  ALL, or DEFAULT (=SVD+CDC+TOP+ARICH).
    * Exmaples:
    * probability of pi hypothesis, using TOP and CDC only =  particleProbability(211, TOP, CDC);
    * probability of K hypothesis, using ARICH, TOP, CDC only =  particleProbability(321, ARICH, TOP, CDC);
    */
    Manager::FunctionPtr particlePairProbability(const std::vector<std::string>& arguments);

    /**
    * @return  returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0.
    * The possible options for the detectors are any combination of {TOP, CDC, SVD, ARICH, ECL, KLM},  ALL, or DEFAULT (=SVD+CDC+TOP+ARICH).
    */
    Manager::FunctionPtr particleMisisngProbability(const std::vector<std::string>& arguments);




    /**
     * return electron Id to be used in the physics analisys
     */
    double particleElectronId(const Particle* part);

    /**
     * return muon Id to be used in the physics analisys
     */
    double particleMuonId(const Particle* part);

    /**
     * return pion Id to be used in the physics analisys
     */
    double particlePionId(const Particle* part);

    /**
     * return kaon Id to be used in the physics analisys
     */
    double particleKaonId(const Particle* part);

    /**
     * return proton Id to be used in the physics analisys
     */
    double particleProtonId(const Particle* part);

    /**
     * return deuteron Id to be used in the physics analisys
     */
    double particleProtonId(const Particle* part);




    /**
     * returns Delta Log L = L(particle's hypothesis) - L(otherHypothesis)
     *
     * all PID subsystems are combined
     */
    double deltaLogL(const Particle* part, Const::ChargedStable& otherHypothesis);

    /**
     * returns Delta Log L = L(particle's hypothesis) - L(pion)
     *
     * all PID subsystems are combined
     */
    double particleDeltaLogLPion(const Particle* part);

    /**
     * returns Delta Log L = L(particle's hypothesis) - L(kaon)
     *
     * all PID subsystems are combined
     */
    double particleDeltaLogLKaon(const Particle* part);

    /**
     * returns Delta Log L = L(particle's hypothesis) - L(proton)
     *
     * all PID subsystems are combined
     */
    double particleDeltaLogLProton(const Particle* part);

    /**
     * returns Delta Log L = L(particle's hypothesis) - L(electron)
     *
     * all PID subsystems are combined
     */
    double particleDeltaLogLElectron(const Particle* part);

    /**
     * returns Delta Log L = L(particle's hypothesis) - L(muon)
     *
     * all PID subsystems are combined
     */
    double particleDeltaLogLMuon(const Particle* part);







    /**
     * return pion vs electron Id
     */
    double particlePionvsElectronId(const Particle* part);

    /**
     * return pion vs electron Id from dEdx measurement
     */
    double particlePionvsElectrondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectrondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectronTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectronARICHId(const Particle* part);

    /**
     * return electron Id
     */
    double particleElectronECLId(const Particle* part);


    /**
     * return electron Id
     */
    double particleMuondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleMuonTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleMuonARICHId(const Particle* part);

    /**
     * return electron Id
     */
    double particlePiondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particlePionTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particlePionARICHId(const Particle* part);


    /**
     * return electron Id
     */
    double particleKaondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleKaonTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleKaonARICHId(const Particle* part);


    /**
     * return electron Id
     */
    double particleProtondEdxId(const Particle* part);

    /**
     * return electron Id
     */
    double particleProtonTOPId(const Particle* part);

    /**
     * return electron Id
     */
    double particleProtonARICHId(const Particle* part);

    /**
     * return 1 if ARICH Id is missing
     */
    double particleMissingARICHId(const Particle*);

    /**
     * return 1 if TOPId is missing
     */
    double particleMissingTOPId(const Particle*);

    /**
     * return 1 if ECLId is missing
     */
    double particleMissingECLId(const Particle* part);

    /**
     * Returns Belle's main PID variable to separate pions, kaons and protons:  atc_pid(3,1,5).prob()
     * Additional arguments are intigers for signal and background hypothesis:
     * (0 = electron, 1 = muon, 2 = pion, 3 = kaon, 4 = proton)
     * This variable should only be used to analyse converted Belle samples. Do not use to analyse Belle II samples.
     */
    double atcPIDBelle(const Particle*, const std::vector<double>& sigAndBkgHyp);
  }
} // Belle2 namespace

