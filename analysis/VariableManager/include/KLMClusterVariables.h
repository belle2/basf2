/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  namespace Variable {

    /**
     * return KLM cluster's timing
     */
    double klmClusterTiming(const Particle* particle);

    /**
     * return KLM cluster's x position
     */
    double klmClusterPositionX(const Particle* particle);

    /**
     * return KLM cluster's y position
     */
    double klmClusterPositionY(const Particle* particle);

    /**
     * return KLM cluster's z position
     */
    double klmClusterPositionZ(const Particle* particle);

    /**
     * return KLM cluster's number of the innermost layer with hits
     */
    double klmClusterInnermostLayer(const Particle* particle);

    /**
     * return KLM cluster's number of layers with hits
     */
    double klmClusterLayers(const Particle* particle);

    /**
     * return the maximum angle of a KLM cluster to this particle in the CMS frame
     */
    double maximumKLMAngleCMS(const Particle* particle);

  }
} // Belle2 namespace

