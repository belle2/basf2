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

void CDCTrack::fillInto(genfit::TrackCand& trackCand) const
{
  // Translate the trajectory information
  if (getFBInfo() == BACKWARD) {
    getEndTrajectory3D().fillInto(trackCand);
  } else {
    getStartTrajectory3D().fillInto(trackCand);
  }

  // Add the hits
  if (getFBInfo() == BACKWARD) {
    bool reverseRLInfo = true;
    fillHitsInto(reverseRange(), trackCand, reverseRLInfo);

  } else {
    bool reverseRLInfo = false;
    fillHitsInto(*this, trackCand, reverseRLInfo);
  }
}






