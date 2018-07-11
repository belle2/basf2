/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
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
    double inCDCAcceptance(const Particle* particle);

    /**
     * particle falls within TOP angular acceptance
     */
    double inTOPAcceptance(const Particle* particle);

    /**
     * particle falls within ARICH angular acceptance
     */
    double inARICHAcceptance(const Particle* particle);

    /**
     * particle falls within ECL angular acceptance
     * 1: Forward; 2: Barrel; 3: Backwards.
     */
    double inECLAcceptance(const Particle* particle);

    /**
     * particle falls within KLM angular acceptance
     * 1: Forward; 2: Barrel; 3: Backwards.
     */
    double inKLMAcceptance(const Particle* particle);

    /**
     * particle falls within BKLM angular acceptance
     */
    double inBKLMAcceptance(const Particle* particle);

    /**
     * particle falls within EKLM angular acceptance
     */
    double inEKLMAcceptance(const Particle* particle);

  }
} // Belle2 namespace

