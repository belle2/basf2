/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/SpacePointLoaderAndPreparer.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

SpacePointLoaderAndPreparer::SpacePointLoaderAndPreparer() : Super()
{
}

void SpacePointLoaderAndPreparer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "SVDSpacePointStoreArrayName"),
                                m_param_SVDSpacePointStoreArrayName,
                                "Name of the SVDSpacePoints Store Array.",
                                m_param_SVDSpacePointStoreArrayName);
}

void SpacePointLoaderAndPreparer::initialize()
{
  Super::initialize();

  m_storeSpacePoints.isRequired(m_param_SVDSpacePointStoreArrayName);
}
