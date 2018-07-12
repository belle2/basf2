/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {
  /** does a tripletFit of the given hits
   * The filter is based on the paper 'A New Three-Dimensional Track Fit with Multiple Scattering'
   * by Andre Schoening et al. https://arxiv.org/abs/1606.04990*/
  class QualityEstimatorTripletFit final : public QualityEstimatorBase {

  public:

    /** Calculating the quality estimate using a triplet fit.
     * @param measurements : vector of SPs of the track candidate
     * @return quality indicator value
     */
    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

    /** perform quality estimation and extract additional information from the fit
     * @param measurements : vector of SPs of the track candidate
     * @return : QualityEstimationResults struct
     */
    virtual QualityEstimationResults estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements) final;

  protected:
    // some variables that are required for the optional results, but are calculated by 'estimateQuality' anyways
    std::vector<double> m_alphas; /**< angle alpha */
    std::vector<double> m_thetas; /**< angle theta */
    std::vector<double> m_R3Ds; /**< 3D radius */
    std::vector<double> m_sigmaR3DSquareds; /**< squared error of 3D radius*/

    double m_averageR3D = 0; /**< average 3D radius */
  };
}

