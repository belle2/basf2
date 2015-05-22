/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/collections/CDCRecoHit3DVector.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FloatType CDCRecoHit3DVector::getSquaredZDist(const CDCTrajectorySZ& trajectorySZ) const
{

  FloatType accumulate = 0;
  for (const_iterator itItem = begin(); itItem != end(); ++itItem) {
    const Item& item = *itItem;
    accumulate += item->getSquaredZDist(trajectorySZ);
  }
  return accumulate;

}
