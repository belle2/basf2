/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

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
     * 1: Forward endcap; 2: Forward overlap; 3: Barrel;
     * 4: Backward overlap; 5: Backward endcap.
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
     * particle falls within the angular region where KLM barrel and endcaps overlap
     */
    double thetaInKLMOverlapAcceptance(const Particle* particle);

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

