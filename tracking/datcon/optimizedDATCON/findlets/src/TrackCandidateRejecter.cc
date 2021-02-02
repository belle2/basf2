/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/TrackCandidateRejecter.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/core/ModuleParamList.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <iostream>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackCandidateRejecter::TrackCandidateRejecter() : Super()
{
}

void TrackCandidateRejecter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

//   moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximumRecursionLevel"),
//                                 m_param_maxRecursionLevel,
//                                 "Maximum recursion level for the fast Hough trafo algorithm.",
//                                 m_param_maxRecursionLevel);

}

void TrackCandidateRejecter::initialize()
{
  Super::initialize();

}

void TrackCandidateRejecter::apply(std::vector<std::vector<const SpacePoint*>>& trackCandidates)
{
  m_prunedTrackCandidates.clear();



}
