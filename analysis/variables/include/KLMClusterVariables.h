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
     * Returns the KlId classifier associated to the KLMCluster.
     * @param[in] particle Input particle.
     */
    double klmClusterKlId(const Particle* particle);

    /**
     * Returns the Belle-style Track flag.
     * @param[in] particle Input particle.
     */
    int klmClusterBelleTrackFlag(const Particle* particle);

    /**
     * Returns the Belle-style ECL flag.
     * @param[in] particle Input particle.
     */
    int klmClusterBelleECLFlag(const Particle* particle);

    /**
     * Returns KLMCluster's timing.
     * @param[in] particle Input particle.
     */
    double klmClusterTiming(const Particle* particle);

    /**
     * Returns KLMCluster's x position.
     * @param[in] particle Input particle.
     */
    double klmClusterPositionX(const Particle* particle);

    /**
     * Returns KLMCluster's y position.
     * @param[in] particle Input particle.
     */
    double klmClusterPositionY(const Particle* particle);

    /**
     * Returns KLMCluster's z position.
     * @param[in] particle Input particle.
     */
    double klmClusterPositionZ(const Particle* particle);

    /**
     * Returns KLMCluster's number of the innermost layer with hits.
     * @param[in] particle Input particle.
     */
    double klmClusterInnermostLayer(const Particle* particle);

    /**
     * Returns KLMCluster's number of layers with hits.
     * @param[in] particle Input particle.
     */
    double klmClusterLayers(const Particle* particle);

    /**
     * Returns KLMCluster's energy (assuming the K_L0 hypothesis).
     * @param[in] particle Input particle.
     */
    double klmClusterEnergy(const Particle* particle);

    /**
     * Returns KLMCluster's momentum magnitude. N.B.: klmClusterMomentum is proportional to klmClusterLayers.
     * @param[in] particle Input particle.
     */
    double klmClusterMomentum(const Particle* particle);

    /**
     * Returns 1 if the associated KLMCluster is in BKLM.
     * @param[in] particle Input particle.
     */
    double klmClusterIsBKLM(const Particle* particle);

    /**
     * Returns 1 if the associated KLMCluster is in EKLM.
     * @param[in] particle Input particle.
     */
    double klmClusterIsEKLM(const Particle* particle);

    /**
     * Returns 1 if the associated KLMCluster is in forward EKLM.
     * @param[in] particle Input particle.
     */
    double klmClusterIsForwardEKLM(const Particle* particle);

    /**
     * Returns 1 if the associated KLMCluster is in backward EKLM.
     * @param[in] particle Input particle.
     */
    double klmClusterIsBackwardEKLM(const Particle* particle);

    /**
     * Returns KLMCluster's theta.
     * @param[in] particle Input particle.
     */
    double klmClusterTheta(const Particle* particle);

    /**
     * Returns KLMCluster's phi.
     * @param[in] particle Input particle.
     */
    double klmClusterPhi(const Particle* particle);

    /**
     * Returns the maximum angle of a KLMCluster to this Particle in the CMS frame.
     * @param[in] particle Input particle.
     */
    double maximumKLMAngleCMS(const Particle* particle);

    /**
     * Returns the number of Tracks matched to the KLMCluster associated to this Particle
     * (>0 for K_L0 and matched Tracks, NaN for not-matched Tracks).
     * @param[in] particle Input particle.
     */
    double nKLMClusterTrackMatches(const Particle* particle);

    /**
     * Returns the number of KLMClusters matched to the Track associated to this Particle.
     * It can return only 0, 1 or NaN (it returns NaN for K_L0 candidates with no Tracks associated).
     * @param[in] particle Input particle.
     */
    double nMatchedKLMClusters(const Particle* particle);

    /**
     * Returns the distance between the Track and the KLMCluster associated to this Particle.
     * This variable returns NaN if there is no Track-to-KLMCluster relationship.
     * @param[in] particle Input particle.
     */
    double klmClusterTrackDistance(const Particle* particle);

  }
} // Belle2 namespace

