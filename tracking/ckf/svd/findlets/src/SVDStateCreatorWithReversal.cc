/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/findlets/SVDStateCreatorWithReversal.h>

#include <tracking/ckf/svd/entities/CKFToSVDState.h>

#include <framework/core/ModuleParamList.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SVDStateCreatorWithReversal::apply(const std::vector<RecoTrack*>& objects, std::vector<CKFToSVDState>& states)
{
  for (const RecoTrack* object : objects) {
    states.emplace_back(object, m_param_reverseSeed);
  }
}

void SVDStateCreatorWithReversal::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter("reverseSeed",
                                m_param_reverseSeed,
                                "Reverse the seed.");
}