/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include "../include/TrackOrientator.h"

#include <boost/foreach.hpp>

#include <framework/logging/Logger.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


TrackOrientator::TrackOrientator()
{
}

TrackOrientator::~TrackOrientator()
{
}


void
TrackOrientator::markOrientation(
  std::vector<CDCTrack>& tracks)
const
{

  BOOST_FOREACH(CDCTrack & track, tracks) {

    //Quick check
    if (track.getStartISuperLayer() > track.getEndISuperLayer()) {

      track.setFBInfo(BACKWARD);

    } else if (track.getStartISuperLayer() < track.getEndISuperLayer()) {

      track.setFBInfo(FORWARD);

    } else {

      //Refined check
      if (track.getStartRecoPos3D().norm() > track.getEndRecoPos3D().norm()) {

        track.setFBInfo(BACKWARD);

      } else if (track.getStartRecoPos3D().norm() < track.getEndRecoPos3D().norm()) {

        track.setFBInfo(FORWARD);

      } else {

        track.setFBInfo(UNKNOWN);

      }

    }
  }

}















