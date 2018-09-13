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
     * returns KLMCluster's timing
     */
    double klmClusterTiming(const Particle* particle);

    /**
     * returns KLMCluster's x position
     */
    double klmClusterPositionX(const Particle* particle);

    /**
     * returns KLMCluster's y position
     */
    double klmClusterPositionY(const Particle* particle);

    /**
     * returns KLMCluster's z position
     */
    double klmClusterPositionZ(const Particle* particle);

    /**
     * returns KLMCluster's number of the innermost layer with hits
     */
    double klmClusterInnermostLayer(const Particle* particle);

    /**
     * returns KLMCluster's number of layers with hits
     */
    double klmClusterLayers(const Particle* particle);

    /**
     * returns KLMCluster's energy (assuming the K_L0 hypothesis). N.B.: if the cluster is built
     * only with RPC hits, the energy is proportional to klmClusterLayers
     */
    double klmClusterEnergy(const Particle* particle);

    /**
     * returns KLMCluster's momentum magnitude. N.B.: if the cluster is built only with RPC hits,
     * the momentum is proportional to klmClusterLayers
     */
    double klmClusterMomentum(const Particle* particle);

    /**
     * returns the maximum angle of a KLMCluster to this Particle in the CMS frame
     */
    double maximumKLMAngleCMS(const Particle* particle);

    /**
     * returns the number of Tracks matched to the KLMCluster associated to this Particle
     * (0 for K_L0, >0 for matched Tracks, NaN for not-matched Tracks).
     */
    double nKLMClusterTrackMatches(const Particle* particle);

    /**
     * returns the number of KLMClusters matched to the Track associated to this Particle.
     * This variable returns NaN for K_L0 (they have no Tracks associated). It can return >1
     */
    double nMatchedKLMClusters(const Particle* particle);

  }
} // Belle2 namespace

