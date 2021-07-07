/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

  m_storeArrayLoader.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "hits"));
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

  B2DEBUG(29, "Starting with " << spacePoints.size() << " hits.");
}
