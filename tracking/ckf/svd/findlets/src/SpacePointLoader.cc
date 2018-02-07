/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/findlets/SpacePointLoader.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

using namespace Belle2;

SpacePointLoader::SpacePointLoader()
{
  addProcessingSignalListener(&m_storeArrayLoader);
}

void SpacePointLoader::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useAssignedHits"), m_param_useAssignedHits,
                                "Use only already assigned hits",
                                m_param_useAssignedHits);
}

void SpacePointLoader::apply(std::vector<const SpacePoint*>& spacePoints)
{
  m_storeArrayLoader.apply(spacePoints);

  if (m_param_useAssignedHits) {
    const auto hitIsNotAlreadyUsed = [](const auto & hit) {
      return not hit->getAssignmentState();
    };
    TrackFindingCDC::erase_remove_if(spacePoints, hitIsNotAlreadyUsed);
  } else {
    const auto hitIsAlreadyUsed = [](const auto & hit) {
      return hit->getAssignmentState();
    };
    TrackFindingCDC::erase_remove_if(spacePoints, hitIsAlreadyUsed);
  }

  B2DEBUG(50, "Starting with " << spacePoints.size() << " hits.");
}
