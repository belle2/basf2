/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/complete/TrackFinderCosmics.h>

#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/core/ModuleParam.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderCosmics::TrackFinderCosmics()
{
  ModuleParamList moduleParamList;
  this->exposeParameters(&moduleParamList, "");
  moduleParamList.getParameter<std::string>("flightTimeEstimation").setDefaultValue("downwards");
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("downwards");
  moduleParamList.getParameter<std::string>("TrackOrientation").setDefaultValue("downwards");
  moduleParamList.getParameter<std::string>("SegmentPairFilter").setDefaultValue("simple");
  moduleParamList.getParameter<std::string>("SegmentPairRelationFilter").setDefaultValue("simple");
}

std::string TrackFinderCosmics::getDescription()
{
  return "Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages - version suitable for cosmics.";
}
