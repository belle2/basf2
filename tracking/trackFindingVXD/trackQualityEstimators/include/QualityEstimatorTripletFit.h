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

    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

    virtual QualityEstimationResults estimateQualityAndProperties(std::vector<SpacePoint const*> const& measurements) final;

  protected:

    // some variables that are required for the optional results, but are calculated by 'estimateQuality' anyways
    std::vector<double> m_alphas;
    std::vector<double> m_thetas;
    std::vector<double> m_R3Ds;
    std::vector<double> m_sigmaR3DSquareds;

    double m_averageR3D = 0;
  };
}

