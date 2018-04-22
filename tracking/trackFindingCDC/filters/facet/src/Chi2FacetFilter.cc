/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/facet/Chi2FacetFilter.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/logging/Logger.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

Chi2FacetFilter::Chi2FacetFilter()
// : m_param_chi2CutByISuperLayer{35.0, 130.0, 73.0, 76.0, 76.0, 84.0, 76.0, 100.0, 110.0} // efficiency 0.99 is a bit to loose
  : m_param_chi2CutByISuperLayer{35.0, 75.0, 75.0, 75.0, 75.0, 75.0, 75.0, 75.0, 75.0}
{
}

Chi2FacetFilter::Chi2FacetFilter(double chi2Cut, double penaltyWidth)
  : m_param_chi2CutByISuperLayer{chi2Cut}
  , m_param_penaltyFactor(penaltyWidth / chi2Cut)
{
}

void Chi2FacetFilter::exposeParameters(ModuleParamList* moduleParamList,
                                       const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "chi2CutByISuperLayer"),
                                m_param_chi2CutByISuperLayer,
                                "Acceptable chi2 values by superlayer id",
                                m_param_chi2CutByISuperLayer);

  moduleParamList->addParameter(prefixed(prefix, "penaltyFactor"),
                                m_param_penaltyFactor,
                                "Factor for cut value to obtain the width used in translation from chi2 values to weight penalties",
                                m_param_penaltyFactor);
}

void Chi2FacetFilter::initialize()
{
  Super::initialize();
  if (m_param_chi2CutByISuperLayer.size() == 1) {
    for (int iSL = 0; iSL < ISuperLayerUtil::c_N; ++iSL) {
      m_chi2CutByISuperLayer[iSL] = m_param_chi2CutByISuperLayer[0];
      m_penaltyWidthByISuperLayer[iSL] = m_param_chi2CutByISuperLayer[0] * m_param_penaltyFactor;
    }
  } else if (m_param_chi2CutByISuperLayer.size() == ISuperLayerUtil::c_N) {
    for (int iSL = 0; iSL < ISuperLayerUtil::c_N; ++iSL) {
      m_chi2CutByISuperLayer[iSL] = m_param_chi2CutByISuperLayer[iSL];
      m_penaltyWidthByISuperLayer[iSL] = m_param_chi2CutByISuperLayer[iSL] * m_param_penaltyFactor;
    }
  } else {
    B2ERROR("Expected chi2CutByISuperLayer to be of length 1 or "
            << ISuperLayerUtil::c_N
            << " received "
            << m_param_chi2CutByISuperLayer.size());
  }

}

Weight Chi2FacetFilter::operator()(const CDCFacet& facet)
{
  constexpr const int nSteps = 1;
  const double chi2 = FacetFitter::fit(facet, nSteps);

  ISuperLayer iSL = facet.getISuperLayer();
  if (chi2 > m_chi2CutByISuperLayer[iSL] or std::isnan(chi2)) {
    return NAN;
  } else {
    // Introducing a mini penilty to distiguish better facets.
    double penalty = std::erf(chi2 / m_penaltyWidthByISuperLayer[iSL]);

    // Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3 - penalty;
  }
}
