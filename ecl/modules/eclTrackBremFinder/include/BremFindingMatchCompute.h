/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth, Patrick Ecker                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

namespace genfit {
  class MeasuredStateOnPlane;
}

namespace Belle2 {

  class ECLCluster;
  class MeasuredStateOnPlane;

  /**
   * Module to compute if an extrapolation to the ECL matches the position of an secondary ECLCLuster
   * to find bremsstrahlung clusters
   */
  class BremFindingMatchCompute {
  public:
    /**
     * Constructor for setting parameters
     */
    BremFindingMatchCompute(float clusterAcceptanceFactor, ECLCluster const& cluster,
                            genfit::MeasuredStateOnPlane const& measuredStateOnPlane) :
      m_clusterAcceptanceFactor(clusterAcceptanceFactor),
      m_eclCluster(cluster),
      m_measuredStateOnPlane(measuredStateOnPlane)
    {}

    /**
     * Check if the angles of the cluster position and the extrapolation match
     */
    bool isMatch();

    /**
     * Return the difference between the angles of extrapolation and cluster position
     */
    double getDistanceHitCluster() {return m_distanceHitCluster;}
  private:
    /**
     * Factor which is multiplied onto the cluster position error to check for matches
     */
    float m_clusterAcceptanceFactor;

    /**
     * Bremsstrahlung cluster candidate gets stored here
     */
    ECLCluster const& m_eclCluster;

    /**
     * VXD hit
     */
    genfit::MeasuredStateOnPlane const& m_measuredStateOnPlane;

    /**
     * Difference between the angles of extrapolation and cluster position
     */
    double m_distanceHitCluster;


  };

} //Belle2
