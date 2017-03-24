/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/facetRelation/Chi2FacetRelationFilter.h>

#include <tracking/trackFindingCDC/fitting/FacetFitter.h>

#include <tracking/trackFindingCDC/geometry/UncertainParameterLine2D.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Chi2FacetRelationFilter::Chi2FacetRelationFilter() = default;

Chi2FacetRelationFilter::Chi2FacetRelationFilter(double chi2Cut, double penaltyWidth)
  : m_param_chi2CutByISuperLayer{chi2Cut,}
  , m_param_penaltyFactor(penaltyWidth / chi2Cut)
{
}

void Chi2FacetRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
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

void Chi2FacetRelationFilter::initialize()
{
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

Weight Chi2FacetRelationFilter::operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet)
{
  if (fromFacet.getStartWire() == toFacet.getEndWire()) return NAN;

  constexpr const int nSteps = 0;
  const UncertainParameterLine2D fitLine = FacetFitter::fit(fromFacet, toFacet, nSteps);
  const double chi2 = fitLine.chi2();

  ISuperLayer iSL = fromFacet.getISuperLayer();
  if (chi2 > m_chi2CutByISuperLayer[iSL] or std::isnan(chi2)) {
    return NAN;
  } else {
    // Introducing a mini penilty to distiguish better facets.
    double penalty = std::erf(chi2 / m_penaltyWidthByISuperLayer[iSL]);

    // The facets contain three hits of the track each.
    // However they have 2 common hits which we have to substract
    // to avoid double counting.
    // Also introduce a small penalty value to distiguish better continuations
    // in the graph
    return -2 - penalty;
  }
}
