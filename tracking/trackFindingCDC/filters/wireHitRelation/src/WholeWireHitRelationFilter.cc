/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHitRelation/WholeWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WholeWireHitRelationFilter::WholeWireHitRelationFilter(int neighborhoodDegree)
  : m_param_degree(neighborhoodDegree)
{
}

void WholeWireHitRelationFilter::exposeParameters(ModuleParamList* moduleParamList,
                                                  const std::string& prefix)
{
  moduleParamList
  ->addParameter(prefixed(prefix, "degree"),
                 m_param_degree,
                 "Neighbor degree which are included. 1 for primary, 2 for secondary, 3 ...",
                 m_param_degree);
}
