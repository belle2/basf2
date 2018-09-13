/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Francesco Tenchini                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {

  namespace Variable {

    /**
     * particle falls within CDC angular acceptance
     */
    double thetaInCDCAcceptance(const Particle* particle);

    /**
     * particle falls within TOP angular acceptance
     */
    double thetaInTOPAcceptance(const Particle* particle);

    /**
     * particle falls within ARICH angular acceptance
     */
    double thetaInARICHAcceptance(const Particle* particle);

    /**
     * particle falls within ECL angular acceptance
     * 1: Forward; 2: Barrel; 3: Backwards.
     */
    double thetaInECLAcceptance(const Particle* particle);

    /**
     * particle falls within BECL angular acceptance
     */
    double thetaInBECLAcceptance(const Particle* particle);

    /**
     * particle falls within EECL angular acceptance
     */
    double thetaInEECLAcceptance(const Particle* particle);

    /**
     * particle falls within KLM angular acceptance
     * 1: Forward; 2: Barrel; 3: Backwards.
     */
    double thetaInKLMAcceptance(const Particle* particle);

    /**
     * particle falls within BKLM angular acceptance
     */
    double thetaInBKLMAcceptance(const Particle* particle);

    /**
     * particle falls within EKLM angular acceptance
     */
    double thetaInEKLMAcceptance(const Particle* particle);

    /**
     * particle falls within TOP transverse momentum acceptance
     */
    double ptInTOPAcceptance(const Particle* particle);

    /**
     * particle falls within Barrel ECL transverse momentum acceptance
     */
    double ptInBECLAcceptance(const Particle* particle);

    /**
     * particle falls within Barrel KLM transverse momentum acceptance
     */
    double ptInBKLMAcceptance(const Particle* particle);

    /**
     * particle falls within CDC geometrical acceptance
     */
    double inCDCAcceptance(const Particle* particle);

    /**
     * particle falls within TOP geometrical acceptance
     */
    double inTOPAcceptance(const Particle* particle);

    /**
     * particle falls within ARICH geometrical acceptance
     */
    double inARICHAcceptance(const Particle* particle);

    /**
     * particle falls within ECL geometrical acceptance
     */
    double inECLAcceptance(const Particle* particle);

    /**
     * particle falls within KLM geometrical acceptance
     */
    double inKLMAcceptance(const Particle* particle);

  }
} // Belle2 namespace

