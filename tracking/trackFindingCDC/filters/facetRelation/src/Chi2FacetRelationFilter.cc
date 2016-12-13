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

Chi2FacetRelationFilter::Chi2FacetRelationFilter(double chi2Cut, double penaltyWidth)
  : m_param_chi2Cut(chi2Cut)
  , m_param_penaltyWidth(penaltyWidth)
{
}

void Chi2FacetRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                               const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "chi2Cut"),
                                m_param_chi2Cut,
                                "Acceptable chi2 fit value",
                                m_param_chi2Cut);
  moduleParamList->addParameter(prefixed(prefix, "penaltyWidth"),
                                m_param_penaltyWidth,
                                "Width used in translation from chi2 values to weight penalties",
                                m_param_penaltyWidth);
}

Weight Chi2FacetRelationFilter::operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet)
{
  if (fromFacet.getStartWire() == toFacet.getEndWire()) return NAN;

  constexpr const int nSteps = 0;
  const UncertainParameterLine2D fitLine = FacetFitter::fit(fromFacet, toFacet, nSteps);
  const double chi2 = fitLine.chi2();

  if (chi2 > m_param_chi2Cut or std::isnan(chi2)) {
    return NAN;
  } else {
    // Introducing a mini penilty to distiguish better facets.
    double penalty = std::erf(chi2 / m_param_penaltyWidth);

    // The facets contain three hits of the track each.
    // However they have 2 common hits which we have to substract
    // to avoid double counting.
    // Also introduce a small penalty value to distiguish better continuations
    // in the graph
    return -2 - penalty;
  }
}
