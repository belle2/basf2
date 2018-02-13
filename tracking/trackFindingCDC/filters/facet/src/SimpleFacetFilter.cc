/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/SimpleFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

SimpleFacetFilter::SimpleFacetFilter()
  : m_param_deviationCosCut(std::cos(M_PI / 180.0 * 9))
{
}

SimpleFacetFilter::SimpleFacetFilter(double deviationCosCut)
  : m_param_deviationCosCut(deviationCosCut)
{
}

void SimpleFacetFilter::exposeParameters(ModuleParamList* moduleParamList,
                                         const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "deviationCosCut"),
                                m_param_deviationCosCut,
                                "Acceptable deviation cosine in the angle of adjacent tangents "
                                "to the drift circles.",
                                m_param_deviationCosCut);
}

Weight SimpleFacetFilter::operator()(const CDCFacet& facet)
{
  facet.adjustFitLine();

  const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
  const ParameterLine2D& startToEnd = facet.getStartToEndLine();
  const ParameterLine2D& middleToEnd = facet.getMiddleToEndLine();

  const double startCos  = startToMiddle.tangential().cosWith(startToEnd.tangential());
  // const double middleCos = startToMiddle.tangential().cosWith(middleToEnd.tangential());
  const double endCos = startToEnd.tangential().cosWith(middleToEnd.tangential());

  // cut on the angle of
  if (startCos > m_param_deviationCosCut and endCos > m_param_deviationCosCut) {
    // Good facet contains three points of the track
    // the amount carried by this facet can the adjusted more realistically
    return 3;
  } else {
    return NAN;
  }
}
