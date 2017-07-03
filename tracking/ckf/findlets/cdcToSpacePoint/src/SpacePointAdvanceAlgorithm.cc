/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/findlets/cdcToSpacePoint/SpacePointAdvanceAlgorithm.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <genfit/MaterialEffects.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <genfit/Exception.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool SpacePointAdvanceAlgorithm::extrapolate(genfit::MeasuredStateOnPlane& measuredStateOnPlane, const SpacePoint* spacePoint) const
{
  // We always use the first cluster here to create the plane. Should not make much difference?
  SVDRecoHit recoHit(spacePoint->getRelated<SVDCluster>());

  // The mSoP plays no role here (it is unused in the function)
  const genfit::SharedPlanePtr& plane = recoHit.constructPlane(measuredStateOnPlane);

  try {
    genfit::MaterialEffects::getInstance()->setNoEffects(not m_param_useMaterialEffects);
    measuredStateOnPlane.extrapolateToPlane(plane);
    genfit::MaterialEffects::getInstance()->setNoEffects(false);
  } catch (genfit::Exception e) {
    B2WARNING(e.what());
    return false;
  }

  return true;
}