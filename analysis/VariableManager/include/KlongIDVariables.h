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

    /** particle Klid. -999 if error */
    double particleKLMKlongID(const Particle* particle);

    /**get Belle stle track flag */
    int particleKLMBelleTrackFlag(const Particle* particle);

    /**get Belle stle ECL flag */
    int particleKLMBelleECLFlag(const Particle* particle);

    /** is particle formard EKLM */
    int particleKLMisForwardEKLM(const Particle* particle);

    /** is particle backward EKLM */
    int particleKLMisBackwardEKLM(const Particle* particle);

    /** is particle BKLM */
    int particleKLMisBKLM(const Particle* particle);

    /**get number of layers */
    float particleKLMgetNLayers(const Particle* particle);

    /** get index of innermost layer*/
    float particleKLMgetInnermostLayer(const Particle* particle);

    /** get energy */
    float particleKLMgetEnergy(const Particle* particle);

    /** get timing */
    float particleKLMgetTime(const Particle* particle);

    /** get theta of the correspondig KLMCluster */
    float particleKLMgetTheta(const Particle* particle);

    /** get phi of the correspondig KLMCluster */
    float particleKLMgetPhi(const Particle* particle);
  }
}
