/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  namespace Variable {

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
     * return electron Id
     */
    double particleElectronId(const Particle* part);

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
     * return muon Id
     */
    double particleMuonId(const Particle* part);

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
     * return pion Id
     */
    double particlePionId(const Particle* part);

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
     * return kaon Id
     */
    double particleKaonId(const Particle* part);

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
     * return proton Id
     */
    double particleProtonId(const Particle* part);

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
  }
} // Belle2 namespace

