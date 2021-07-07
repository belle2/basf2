/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {
  /** does a tripletFit of the given hits
   * The filter is based on the paper 'A New Three-Dimensional Track Fit with Multiple Scattering'
   * by Andre Schoening et al. https://arxiv.org/abs/1606.04990*/
  class QualityEstimatorTripletFit final : public QualityEstimatorBase {

  public:
    /** Constructor
     */
    QualityEstimatorTripletFit(double materialBudgetFactor = 1.45,
                               double maxPt = 0.01):
      QualityEstimatorBase()
    {
      m_maxPt = maxPt;
      m_materialBudgetFactor = materialBudgetFactor;
    };

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

    /// Triplet Fit hyper parameters

    /** Scaling factor for material budget which is applied to the radiation length
     *  value X/X_0 = 0.57% which is taken from the Belle II TDR page 156.
     *  This scaling factor is optimized to achieve the best performance on MC.
     */
    double m_materialBudgetFactor = 1.45;

    /** Using average material budged of SVD sensors for approximation of radiation length
     *  Belle II TDR page 156 states a value of 0.57% X_0.
     *  This approximation is a first approach to the problem and must be checked.
     */
    double m_averageRadLength = 0.0057;

    /** Cut off value for 3D Radius calculated in Triplet Fit.
     *  This value is a hyper parameter of the Triplet Fit which is tuned on MC.
     */
    double m_maxPt = 0.01;
  };
}

