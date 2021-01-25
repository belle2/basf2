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

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/geometry/B2Vector3.h>

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

void SpacePointLoaderAndPreparer::apply(std::vector<std::tuple<const SpacePoint*, const VxdID, double, double, double>>& hits)
{
  if (m_storeSpacePoints.getEntries() == 0) return;

  for (auto& spacePoint : m_storeSpacePoints) {
    const B2Vector3D& hitPos = spacePoint.getPosition();
    const double hitRadiusSquared = hitPos.Perp() * hitPos.Perp();
    // content of hits: pointer to the SpacePoint, its VxdID, the conformal-transformed x and y coordinates of the SpacePoint, and its z coordinate
    hits.emplace_back(&spacePoint, spacePoint.getVxdID(), 2.*hitPos.X() / hitRadiusSquared, 2.*hitPos.Y() / hitRadiusSquared,
                      hitPos.Z());
  }

}
