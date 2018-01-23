/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/SimpleFacetRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SimpleFacetRelationFilter::SimpleFacetRelationFilter()
  : m_param_deviationCosCut(cos(0.4))
{
}

SimpleFacetRelationFilter::SimpleFacetRelationFilter(double deviationCosCut)
  : m_param_deviationCosCut(deviationCosCut)
{
}

void SimpleFacetRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "deviationCosCut"),
                                m_param_deviationCosCut,
                                "Acceptable deviation cosine in the angle of adjacent tangents "
                                "to the drift circles.",
                                m_param_deviationCosCut);
}

Weight SimpleFacetRelationFilter::operator()(const CDCFacet& fromFacet,
                                             const CDCFacet& toFacet)
{
  if (fromFacet.getStartWireHit().isOnWire(toFacet.getEndWire())) return NAN;

  // the compatibility of the short legs or all?
  // start end to continuation middle end
  // start middle to continuation start end

  const ParameterLine2D& fromStartToMiddle = fromFacet.getStartToMiddleLine();
  const ParameterLine2D& fromStartToEnd = fromFacet.getStartToEndLine();

  const ParameterLine2D& toStartToEnd = toFacet.getStartToEndLine();
  const ParameterLine2D& toMiddleToEnd = toFacet.getMiddleToEndLine();

  const double fromMiddleCos = fromStartToMiddle.tangential().cosWith(toStartToEnd.tangential());
  const double toMiddleCos = fromStartToEnd.tangential().cosWith(toMiddleToEnd.tangential());

  // check both
  if (fromMiddleCos > m_param_deviationCosCut and toMiddleCos > m_param_deviationCosCut) {
    // the weight must be -2 because the overlap of the facets is two points
    // so the amount of two facets is 4 points hence the cellular automat
    // must calculate 3 + (-2) + 3 = 4 as cellstate
    // this can of course be adjusted for a more realistic information measure
    // ( together with the facet creator filter)
    return -2;
  } else {
    return NAN;
  }
}
