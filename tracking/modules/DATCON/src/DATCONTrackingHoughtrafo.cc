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
  int cnt;
  int strip_id;
  double r;
  double rStrip;
  svdClusterMapIter iter;
  svdClusterPair clusterInfo;
  TVector3 pos;
  TVector2 hough, center;
  VxdID sensorID;

  cnt = 0;
  center.Set(m_trackCenterX, m_trackCenterY);

  for (iter = mapClusters.begin(); iter != mapClusters.end(); ++iter) {
    strip_id = iter->first;
    clusterInfo = iter->second;
    pos = clusterInfo.second;
    sensorID = clusterInfo.first;
    if (!u_side) {
      if (m_usePhase2Simulation) {
        hough.Set(pos.X(), pos.Z());
      } else {
        r = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
        rStrip = r;
        hough.Set(rStrip, pos.Z());
      }
      vHough.insert(make_pair(strip_id, make_pair(sensorID, hough)));
    } else {
      if (conformal) {
        if (m_usePhase2Simulation) {
          r = sqrt((pow(pos.X() - center.X(), 2.0) + pow(pos.Y() - center.Y(), 2.0)));
          rStrip = r;
        } else {
          r = sqrt(pos.X() * pos.X() + pos.Y() * pos.Y());
          rStrip = r;
        }
        if (m_xyHoughUside) {
          hough.Set(pos.X() / (rStrip * rStrip), pos.Y() / (rStrip * rStrip));
        } else if (m_rphiHoughUside) {
          hough.Set(pos.Phi() - M_PI / 2.0, rStrip);
        }
      } else {
        hough.Set(pos.X(), pos.Y());
      }
      uHough.insert(make_pair(strip_id, make_pair(sensorID, hough)));
    }
    ++cnt;
  }

  /* Write hough space into gnuplot file */
  if (m_writeHoughSpace) {
    houghTrafoPlot(u_side);
  }
}
