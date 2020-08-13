/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Schnell, Christian Wessel                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/DATCON/DATCONTrackingModule.h>

using namespace std;
using namespace Belle2;

/*
* Transform to Hough space
*/
void
DATCONTrackingModule::houghTrafo2d(svdHitMap& mapClusters, bool u_side)
{
  TVector2 center;

  center.Set(m_trackCenterX, m_trackCenterY);

  for (auto& iter : mapClusters) {
    double rStrip = 0;
    int hitID = iter.first;
    VxdID sensorID = iter.second.first;
    TVector3 pos = iter.second.second;
    TVector2 hough;

    if (!u_side) {
      if (m_usePhase2Simulation) {
        hough.Set(pos.X(), pos.Z());
      } else {
        rStrip = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
        hough.Set(rStrip, pos.Z());
      }
      vHough.insert(make_pair(hitID, make_pair(sensorID, hough)));
    } else {
      if (m_usePhase2Simulation) {
        rStrip = sqrt((pow(pos.X() - center.X(), 2.0) + pow(pos.Y() - center.Y(), 2.0)));
      } else {
        rStrip = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
      }
      hough.Set(pos.X() / (rStrip * rStrip), pos.Y() / (rStrip * rStrip));
      uHough.insert(make_pair(hitID, make_pair(sensorID, hough)));
    }
  }
}
