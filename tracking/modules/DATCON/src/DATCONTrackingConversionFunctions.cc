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

void DATCONTrackingModule::prepareDATCONSVDSpacePoints()
{
  int indexCounter = 0;

  if (storeDATCONSVDSpacePoints.getEntries() == 0) {
    return;
  }

  uClusters.clear();
  vClusters.clear();

  /* First convert to absolute hits and save into a map */
  for (auto& datconspacepoint : storeDATCONSVDSpacePoints) {
    VxdID sensorID = datconspacepoint.getVxdID();
    TVector3 pos = datconspacepoint.getPosition();

    unsigned int uID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
                       indexCounter;
    unsigned int vID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
                       indexCounter;
    vClusters.insert(std::make_pair(vID, std::make_pair(sensorID, pos)));
    uClusters.insert(std::make_pair(uID, std::make_pair(sensorID, pos)));
    ++indexCounter;

  }
}

void DATCONTrackingModule::prepareSVDSpacePoints()
{
  int indexCounter = 0;

  if (storeSVDSpacePoints.getEntries() == 0) {
    return;
  }

  uClusters.clear();
  vClusters.clear();

  /* First convert to absolute hits and save into a map */
  for (auto& svdspacepoint : storeSVDSpacePoints) {
    VxdID sensorID = svdspacepoint.getVxdID();
    TVector3 pos = svdspacepoint.getPosition();

    unsigned int uID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
                       indexCounter;
    unsigned int vID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
                       indexCounter;
    vClusters.insert(std::make_pair(vID, std::make_pair(sensorID, pos)));
    uClusters.insert(std::make_pair(uID, std::make_pair(sensorID, pos)));
    ++indexCounter;

  }
}
