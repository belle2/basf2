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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void SimpleFacetRelationFilter::setParameter(const std::string& key, const std::string& value)
{
  if (key == "deviation_cos_cut") {
    m_param_deviationCosCut = stod(value);
    B2INFO("Filter received parameter '" << key << "' " << value);
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> SimpleFacetRelationFilter::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["deviation_cos_cut"] = "Acceptable deviation cosine in the angle of adjacent tangents to the "
                             "drift circles.";
  return des;
}



Weight SimpleFacetRelationFilter::operator()(const CDCFacet& fromFacet,
                                             const CDCFacet& toFacet)
{

  if (fromFacet.getStartWire() == toFacet.getEndWire()) return NAN;

  // the compatibility of the short legs or all?
  // start end to continuation middle end
  // start middle to continuation start end

  const ParameterLine2D& fromStartToMiddle = fromFacet.getStartToMiddleLine();
  const ParameterLine2D& fromStartToEnd    = fromFacet.getStartToEndLine();

  const ParameterLine2D& toStartToEnd   = toFacet.getStartToEndLine();
  const ParameterLine2D& toMiddleToEnd  = toFacet.getMiddleToEndLine();

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
