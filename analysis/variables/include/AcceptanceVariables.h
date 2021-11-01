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
    bool thetaInCDCAcceptance(const Particle* particle);

    /**
     * particle falls within TOP angular acceptance
     */
    bool thetaInTOPAcceptance(const Particle* particle);

    /**
     * particle falls within ARICH angular acceptance
     */
    bool thetaInARICHAcceptance(const Particle* particle);

    /**
     * particle falls within ECL angular acceptance
     * 1: Forward; 2: Barrel; 3: Backwards.
     */
    int thetaInECLAcceptance(const Particle* particle);

    /**
     * particle falls within BECL angular acceptance
     */
    bool thetaInBECLAcceptance(const Particle* particle);

    /**
     * particle falls within EECL angular acceptance
     */
    bool thetaInEECLAcceptance(const Particle* particle);

    /**
     * particle falls within KLM angular acceptance
     * 1: Forward endcap; 2: Forward overlap; 3: Barrel;
     * 4: Backward overlap; 5: Backward endcap.
     */
    int thetaInKLMAcceptance(const Particle* particle);

    /**
     * particle falls within BKLM angular acceptance
     */
    bool thetaInBKLMAcceptance(const Particle* particle);

    /**
     * particle falls within EKLM angular acceptance
     */
    bool thetaInEKLMAcceptance(const Particle* particle);

    /**
     * particle falls within the angular region where KLM barrel and endcaps overlap
     */
    bool thetaInKLMOverlapAcceptance(const Particle* particle);

    /**
     * particle falls within TOP transverse momentum acceptance
     */
    bool ptInTOPAcceptance(const Particle* particle);

    /**
     * particle falls within Barrel ECL transverse momentum acceptance
     */
    bool ptInBECLAcceptance(const Particle* particle);

    /**
     * particle falls within Barrel KLM transverse momentum acceptance
     */
    bool ptInBKLMAcceptance(const Particle* particle);

    /**
     * particle falls within CDC geometrical acceptance
     */
    bool inCDCAcceptance(const Particle* particle);

    /**
     * particle falls within TOP geometrical acceptance
     */
    bool inTOPAcceptance(const Particle* particle);

    /**
     * particle falls within ARICH geometrical acceptance
     */
    bool inARICHAcceptance(const Particle* particle);

    /**
     * particle falls within ECL geometrical acceptance
     */
    bool inECLAcceptance(const Particle* particle);

    /**
     * particle falls within KLM geometrical acceptance
     */
    bool inKLMAcceptance(const Particle* particle);

  }
} // Belle2 namespace

