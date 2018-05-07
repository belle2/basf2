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

  VxdID sensorID;
  TVector3 pos;
  unsigned int uID, vID;
  double uCoordinate, vCoordinate;
  int uCellID, vCellID;
  int indexCounter = 0;

  if (storeHoughCluster.isValid()) {
    storeHoughCluster.clear();
  }

  if (storeDATCONSVDSpacePoints.getEntries() == 0) {
    return;
  }

  indexU = 0;
  indexV = 0;
  uClusters.clear();
  vClusters.clear();

  /* First convert to absolute hits and save into a map */
  for (auto& datconspacepoint : storeDATCONSVDSpacePoints) {
    sensorID = datconspacepoint.getVxdID();
    pos = datconspacepoint.getPosition();

    if (m_countStrips) {
      uID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
            indexCounter;
      vID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
            indexCounter;
      vClusters.insert(std::make_pair(vID, std::make_pair(sensorID, pos)));
      uClusters.insert(std::make_pair(uID, std::make_pair(sensorID, pos)));
      ++indexU;
      ++indexV;
      ++indexCounter;
    } else {
      pair<double, double> datconnormloccoord = make_pair(datconspacepoint.getNormalizedLocalU(), datconspacepoint.getNormalizedLocalV());
      pair<double, double> datconloccoord = datconspacepoint.convertNormalizedToLocalCoordinates(datconnormloccoord, sensorID);
      uCoordinate = datconloccoord.first;
      vCoordinate = datconloccoord.second;
      const SVD::SensorInfo* currentSensor = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
      uCellID = currentSensor->getUCellID(uCoordinate, vCoordinate, false);
      vCellID = currentSensor->getVCellID(vCoordinate, false);
      uCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      vCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      vClusters.insert(std::make_pair(vCellID, std::make_pair(sensorID, pos)));
      uClusters.insert(std::make_pair(uCellID, std::make_pair(sensorID, pos)));
      ++indexU;
      ++indexV;
    }
    storeHoughCluster.appendNew(DATCONHoughCluster(sensorID, pos));
  }
}



void DATCONTrackingModule::prepareSVDSpacePoints()
{

  VxdID sensorID;
  TVector3 pos, local_pos;
  unsigned int uID, vID;
  double uCoordinate, vCoordinate;
  int uCellID, vCellID;
  int indexCounter = 0;

  if (storeHoughCluster.isValid()) {
    storeHoughCluster.clear();
  }

  if (storeSVDSpacePoints.getEntries() == 0) {
    return;
  }

  indexU = 0;
  indexV = 0;
  uClusters.clear();
  vClusters.clear();

  /* First convert to absolute hits and save into a map */
  for (auto& svdspacepoint : storeSVDSpacePoints) {
    sensorID = svdspacepoint.getVxdID();
    pos = svdspacepoint.getPosition();

    if (m_countStrips) {
      uID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
            indexCounter;
      vID = 10000000 * sensorID.getLayerNumber() + 100000 * sensorID.getLadderNumber() + 10000 * sensorID.getSensorNumber() +
            indexCounter;
      vClusters.insert(std::make_pair(vID, std::make_pair(sensorID, pos)));
      uClusters.insert(std::make_pair(uID, std::make_pair(sensorID, pos)));
      ++indexU;
      ++indexV;
      ++indexCounter;
    } else {
      pair<double, double> svdnormloccoord = make_pair(svdspacepoint.getNormalizedLocalU(), svdspacepoint.getNormalizedLocalV());
      pair<double, double> svdloccoord = svdspacepoint.convertNormalizedToLocalCoordinates(svdnormloccoord, sensorID);
      uCoordinate = svdloccoord.first;
      vCoordinate = svdloccoord.second;
      const SVD::SensorInfo* currentSensor = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(sensorID));
      uCellID = currentSensor->getUCellID(uCoordinate, vCoordinate, false);
      vCellID = currentSensor->getVCellID(vCoordinate, false);
      uCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      vCellID += 1000000 * sensorID.getLayerNumber() + 10000 * sensorID.getLadderNumber() + 1000 * sensorID.getSensorNumber();
      vClusters.insert(std::make_pair(vCellID, std::make_pair(sensorID, pos)));
      uClusters.insert(std::make_pair(uCellID, std::make_pair(sensorID, pos)));
      ++indexU;
      ++indexV;
    }
    storeHoughCluster.appendNew(DATCONHoughCluster(sensorID, pos));
  }
}

