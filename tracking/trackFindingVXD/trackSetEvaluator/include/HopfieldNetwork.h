/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

//basf2
#include <tracking/trackFindingVXD/trackSetEvaluator/OverlapResolverNodeInfo.h>


namespace Belle2 {

  /** Hopfield Algorithm with number based inputs.
   *
   *  This class encapsulates the actual algorithm with pure number inputs
   *  (instead of objects of TrackCandidates or whatever).
   *  The reason for this is mainly better testability and potentially as well
   *  reusability.
   *  The development of this class started as a copy from Jakob's implementation for
   *  the VXDTF2.
   *
   *  Relevant resources:
   *  [1] R. Frühwirth, "Selection of optimal subsets of tracks with a feedback neural network",
   *  Comput. Phys. Commun., vol. 78, pp. 23–28, 1993.
   */
  class HopfieldNetwork {
  public:
    /** Constructor taking parameters for the algorithm.
     *
     *  @param omega  Should be between 0 and 1; small values lead to large number of nodes, large ones to large sums of quality indicators.
     *  @param T      Temperature for annealing.
     *  @param Tmin   Minimal reached temperature in the annealing scheme.
     *  @param cmax   Maximum change of weights between iterations, so we accept the network as converged.
     */
    HopfieldNetwork(float omega = 0.5, float T = 3.1, float Tmin = 0.1, float cmax = 0.01):
      m_omega(omega), m_T(T), m_Tmin(Tmin), m_cmax(cmax)
    {}

    /** Performance of the actual algorithm.
     *
     *  HINT FOR SPEED OPTIMIZATION: A lot of time is spend for checking the Logger,
     *  if you don't have LOG_NO_B2DEBUG defined. If you have done that,
     *  a lot of time is taken by the tanh function and drawing random
     *  numbers.<br>
     *
     *  Currently this algorithm can only be used once for each instance,
     *  as the algorithm parameter variables are changed during the performance.
     *  @sa OverlapResolverNodeInfo
     */
    unsigned short doHopfield(std::vector<OverlapResolverNodeInfo>& overlapResolverNodeInfos, unsigned short nIterations = 20);

  private:
    float m_omega; /**< tuning parameter of the hopfield network */
    float m_T; /**< start temperature of annealing */
    float m_Tmin; /**< minimal temperature allowed */
    float m_cmax; /**< maximal change of weights between iterations */

  };
}
