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
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

Chi2FacetFilter::Chi2FacetFilter()
{
}

Chi2FacetFilter::Chi2FacetFilter(double chi2Cut,
                                 double penaltyWidth):
  m_param_chi2Cut(chi2Cut),
  m_param_penaltyWidth(penaltyWidth)
{
}

void Chi2FacetFilter::exposeParameters(ModuleParamList* moduleParamList,
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

Weight Chi2FacetFilter::operator()(const CDCFacet& facet)
{
  const bool nSteps = 1;
  double chi2 = FacetFitter::fit(facet, nSteps);
  if (chi2 > m_param_chi2Cut or std::isnan(chi2)) {
    return NAN;
  } else {

    // Introducing a mini penilty to distiguish better facets.
    double penalty = std::erf(chi2 / m_param_penaltyWidth);

    // Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3 - penalty;
  }
}
