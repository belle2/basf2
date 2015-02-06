/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCTrack.h"

#include <tracking/trackFindingCDC/eventdata/collections/FillGenfitTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace genfit;

TRACKFINDINGCDC_SwitchableClassImp(CDCTrack)

bool CDCTrack::fillInto(genfit::TrackCand& trackCand) const
{
  // Add the hits
  if (getFBInfo() == BACKWARD) {
    bool reverseRLInfo = true;
    fillHitsInto(reverseRange(), trackCand, reverseRLInfo);

  } else {
    bool reverseRLInfo = false;
    fillHitsInto(*this, trackCand, reverseRLInfo);
  }

  // Translate the trajectory information
  if (getFBInfo() == BACKWARD) {
    CDCTrajectory3D endTrajectory3D = getEndTrajectory3D().reversed();
    return endTrajectory3D.fillInto(trackCand);
  } else {
    return getStartTrajectory3D().fillInto(trackCand);
  }

}






