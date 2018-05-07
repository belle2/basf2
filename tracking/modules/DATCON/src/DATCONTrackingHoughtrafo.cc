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
DATCONTrackingModule::houghTrafo2d(svdClusterMap& mapClusters, bool u_side, bool conformal = false)
{
  int hitID;
  double rStrip;
  TVector3 pos;
  TVector2 hough, center;
  VxdID sensorID;

  center.Set(m_trackCenterX, m_trackCenterY);

  for (auto& iter : mapClusters) {
    hitID = iter.first;
    sensorID = iter.second.first;
    pos = iter.second.second;
    if (!u_side) {
      if (m_usePhase2Simulation) {
        hough.Set(pos.X(), pos.Z());
      } else {
        rStrip = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
        hough.Set(rStrip, pos.Z());
      }
      vHough.insert(make_pair(hitID, make_pair(sensorID, hough)));
    } else {
      if (conformal) {
        if (m_usePhase2Simulation) {
          rStrip = sqrt((pow(pos.X() - center.X(), 2.0) + pow(pos.Y() - center.Y(), 2.0)));
        } else {
          rStrip = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
        }
        if (m_xyHoughUside) {
          hough.Set(pos.X() / (rStrip * rStrip), pos.Y() / (rStrip * rStrip));
        } else if (m_rphiHoughUside) {
          hough.Set(pos.Phi() - M_PI / 2.0, rStrip);
        }
      } else {
        hough.Set(pos.X(), pos.Y());
      }
      uHough.insert(make_pair(hitID, make_pair(sensorID, hough)));
    }
  }
}
