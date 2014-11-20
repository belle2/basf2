/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Moritz Nadler                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/VXDTFHelperTools/EventCounterModule.h"
#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <map>

#include <algorithm>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventCounter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventCounterModule::EventCounterModule() : Module()
{
  InitializeCounters();

  //Set module properties
  setDescription("Highlights current event and does some minimal statistics for VXD Clusters");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("stepSize", m_PARAMstepSize, "e.g. 100 will highlight every 100th event", int(100));
  addParam("showClusterStatistics", m_PARAMshowClusterStatistics, "if activated, some statistics for SVD and PXD Clusters are measured", bool(true));
}



void EventCounterModule::initialize()
{
  StoreArray<PXDCluster>::optional();
  StoreArray<SVDCluster>::optional();
}



void EventCounterModule::beginRun()
{
  B2INFO("################## eventCounter enabled, highlighting every " << m_PARAMstepSize << " event ######################\n" << "extended infos you can get by activating parameter 'showClusterStatistics', and/or setting debug level to 1 (runWise extra) or 2 (event wise extra)")

  InitializeCounters();

  if (m_PARAMshowClusterStatistics == false) { return; }

  // filling unused entries of m_countedPixelsAndStrips with dummy data to prevent division by zero lateron
  m_countedPixelsAndStrips.m_pxdClusterCounter.fill(1);
  m_countedPixelsAndStrips.m_svdClusterCounter.fill(1);
  m_countedPixelsAndStrips.m_svdClusterCombinationsCounter.fill(1);

  unsigned int knownLayers = 0, knownLadders = 0, knownSensors = 0, nUnknownLayers = 0, nUnknownLadders = 0, nUnknownSensors = 0, pos = 0; // pos is position in corresponding array in the countedStuff instance


  // import information of number of strips/pixels for each sensor in the VXD (unrecognized detector types are ignored (like Tel)):
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  set< VxdID > layers = aGeometry.getLayers();
  for (VxdID layer : layers) {
    bool isPXD = false, isSVD = false;
    if (layer.getLayerNumber() >= c_nInnermostPXDLayer and layer.getLayerNumber() < c_nInnermostPXDLayer + c_nPXDLayers) {
      isPXD = true;
      pos = layer.getLayerNumber() - c_nInnermostPXDLayer;
    }
    if (layer.getLayerNumber() >= c_nInnermostSVDLayer and layer.getLayerNumber() < c_nInnermostSVDLayer + c_nSVDLayers) {
      isSVD = true;
      pos = layer.getLayerNumber() - c_nInnermostSVDLayer;
    }

    if (isPXD == false and isSVD == false) { nUnknownLayers++; } else { knownLayers++; }
    if (isPXD == true and isSVD == true) { B2ERROR("eventCounterModule: found layer which is SVD and PXD sensor at the same time! LayerNumbe: " << layer.getLayerNumber()); }

    const set<VxdID>& ladders = aGeometry.getLadders(layer);
    for (VxdID ladder : ladders) {
      const set<VxdID>& sensors = aGeometry.getSensors(ladder);

      if (isPXD == false and isSVD == false) { nUnknownLadders++; } else { knownLadders++; }
      for (VxdID sensor : sensors) {
        const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(sensor);

        if (isPXD == false and isSVD == false) { nUnknownSensors++; continue; } else { knownSensors++; }

        // nPixels@Sensor uPixels x vPixels. nStrips@Sensor uStrips + vStrips:
        if (isPXD == true) {
          m_countedPixelsAndStrips.m_pxdPixelCounter.at(pos) += (aSensorInfo.getUCells() * aSensorInfo.getVCells());
        }
        if (isSVD == true) {
          m_countedPixelsAndStrips.m_svdStripCounter4U.at(pos) += aSensorInfo.getUCells();
          m_countedPixelsAndStrips.m_svdStripCounter4V.at(pos) += aSensorInfo.getVCells();
          m_countedPixelsAndStrips.m_svdStripCounterTotal.at(pos) += (aSensorInfo.getUCells() + aSensorInfo.getVCells());
        }
        B2DEBUG(1, " sensor " << sensor << " has got " << aSensorInfo.getUCells() << "/" << aSensorInfo.getVCells() << " u/v-pixels/strips")
      }
    }
  }

  B2INFO("eventCounter - beginRun with m_PARAMshowClusterStatistics == true. Total number of Pixels per PXDsensor and u/vStrips per SVDSensor:\n" << m_countedPixelsAndStrips.PrintStuff(false) << "counted number of known and unrecognized layers/ladders/sensors: " << knownLayers << "/" << knownLadders << "/" << knownSensors << ", " << nUnknownLayers << "/" << nUnknownLadders << "/" << nUnknownSensors)
}



void EventCounterModule::event()
{
  m_eventCounter++;

  if (m_PARAMshowClusterStatistics == false) {
    if (m_eventCounter % m_PARAMstepSize == 0) { B2INFO("EventCounterModule - Event: " << m_eventCounter) }
    else { B2DEBUG(2, "EventCounterModule - Event: " << m_eventCounter) }
    return;
  }


  StoreArray<PXDCluster> aPxdClusterArray;
  m_pxdClusterCounter += aPxdClusterArray.getEntries();

  StoreArray<SVDCluster> aSvdClusterArray;
  m_svdClusterCounter += aSvdClusterArray.getEntries();

  CountStuff<unsigned int> thisEventCounted;

  vector<unsigned short>
  svdSensorIDsOccured,// logs occured SVD sensorIDs for this event
  pxdSensorIDsOccured; // logs occured PXD sensorIDs for this event
  map<unsigned short, unsigned short>
  sensorsAndClustersU,// logs for each occured sensor (.first), logs number of occured U clusters (SVD only) (.second)
  sensorsAndClustersV,// logs for each occured sensor (.first), logs number of occured V clusters (SVD only) (.second)
  sensorsAndClusters, // logs for each occured sensor (.first), logs number of occured clusters (.second)
  sensorsAndStripsU,// logs for each occured sensor (.first), logs number of illuminated U strips (SVD only) (.second)
  sensorsAndStripsV,// logs for each occured sensor (.first), logs number of illuminated V strips (SVD only) (.second)
  sensorsAndPixels; // logs for each occured sensor (.first), logs number of illuminated pixels (.second)


  /** Clusters: accepts a sensorID and searches for existing entries (which are increased, if found or created if not) */
  auto lambdaCheckSVDSensor4Clusters = [&svdSensorIDsOccured, &sensorsAndClustersU, &sensorsAndClustersV](unsigned short iD, bool isUCluster) {
    auto foundPos = std::find(svdSensorIDsOccured.begin(), svdSensorIDsOccured.end(), iD);
    if (foundPos == svdSensorIDsOccured.end()) { svdSensorIDsOccured.push_back(iD); }

    if (isUCluster == true) {
      auto foundU = sensorsAndClustersU.find(iD);
      if (foundU == sensorsAndClustersU.end()) { sensorsAndClustersU.insert({iD, 1}); }
      else { foundU->second += 1; }
      return;
    }
    auto foundV = sensorsAndClustersV.find(iD);
    if (foundV == sensorsAndClustersV.end()) { sensorsAndClustersV.insert({iD, 1}); }
    else { foundV->second += 1; }
  };


  /** Clusters: accepts a sensorID and searches for existing entries (which are increased, if found or created if not) */
  auto lambdaCheckPXDSensor4Clusters = [&pxdSensorIDsOccured, &sensorsAndClusters](unsigned short iD) {
    auto foundPos = std::find(pxdSensorIDsOccured.begin(), pxdSensorIDsOccured.end(), iD);
    if (foundPos == pxdSensorIDsOccured.end()) { pxdSensorIDsOccured.push_back(iD); }

    auto found = sensorsAndClusters.find(iD);
    if (found == sensorsAndClusters.end()) { sensorsAndClusters.insert({iD, 1}); }
    else { found->second += 1; }
  };


  /** Sptrips: accepts a sensorID and searches for existing entries (which are increased, if found or created if not) */
  auto lambdaCheckSVDSensor4Strips = [&sensorsAndStripsU, &sensorsAndStripsV](unsigned short iD, bool isUCluster, unsigned short size) {
    if (isUCluster == true) {
      auto foundU = sensorsAndStripsU.find(iD);
      if (foundU == sensorsAndStripsU.end()) { sensorsAndStripsU.insert({iD, size}); }
      else { foundU->second += size; }
      return;
    }
    auto foundV = sensorsAndStripsV.find(iD);
    if (foundV == sensorsAndStripsV.end()) { sensorsAndStripsV.insert({iD, size}); }
    else { foundV->second += size; }
  };


  /** Pixels: accepts a sensorID and searches for existing entries (which are increased, if found or created if not) */
  auto lambdaCheckPXDSensor4Pixels = [&sensorsAndPixels](unsigned short iD, unsigned short size) {
    auto found = sensorsAndPixels.find(iD);
    if (found == sensorsAndPixels.end()) { sensorsAndPixels.insert({iD, size}); }
    else { found->second += size; }
  };


  // collect Infos from PXD and SVD
  for (const PXDCluster & aCluster : aPxdClusterArray) {
    lambdaCheckPXDSensor4Clusters(aCluster.getSensorID());
    lambdaCheckPXDSensor4Pixels(aCluster.getSensorID(), aCluster.getSize());
  }
  for (const SVDCluster & aCluster : aSvdClusterArray) {
    lambdaCheckSVDSensor4Clusters(aCluster.getRawSensorID(), aCluster.isUCluster());
    lambdaCheckSVDSensor4Strips(aCluster.getRawSensorID(), aCluster.isUCluster(), aCluster.getSize());
  }


  // PXD:
  for (auto & infoPair : sensorsAndClusters) {
    unsigned short layerID = VxdID(infoPair.first).getLayerNumber();

    thisEventCounted.m_pxdClusterCounter.at(layerID - c_nInnermostPXDLayer) += infoPair.second;
  }
  for (auto & infoPair : sensorsAndPixels) {
    unsigned short layerID = VxdID(infoPair.first).getLayerNumber();

    thisEventCounted.m_pxdPixelCounter.at(layerID - c_nInnermostPXDLayer) += infoPair.second;
  }


  //SVD:
  for (unsigned short aSensor : svdSensorIDsOccured) {
    unsigned short layerID = VxdID(aSensor).getLayerNumber();
    unsigned short uClustersOfSensor = 0, vClustersOfSensor = 0, uStripsOfSensor = 0, vStripsOfSensor = 0;

    auto found = sensorsAndClustersU.find(aSensor);
    if (found != sensorsAndClustersU.end()) { uClustersOfSensor = found->second ; }
    found = sensorsAndClustersV.find(aSensor);
    if (found != sensorsAndClustersV.end()) { vClustersOfSensor = found->second ; }
    found = sensorsAndStripsU.find(aSensor);
    if (found != sensorsAndStripsU.end()) { uStripsOfSensor = found->second ; }
    found = sensorsAndStripsV.find(aSensor);
    if (found != sensorsAndStripsV.end()) { vStripsOfSensor = found->second ; }

    thisEventCounted.m_svdStripCounter4U.at(layerID - c_nInnermostSVDLayer) += uStripsOfSensor;
    thisEventCounted.m_svdStripCounter4V.at(layerID - c_nInnermostSVDLayer) += vStripsOfSensor;
    thisEventCounted.m_svdStripCounterTotal.at(layerID - c_nInnermostSVDLayer) += (uStripsOfSensor + vStripsOfSensor);
    thisEventCounted.m_svdClusterCounter.at(layerID - c_nInnermostSVDLayer) += (uClustersOfSensor + vClustersOfSensor);
    thisEventCounted.m_svdClusterCombinationsCounter.at(layerID - c_nInnermostSVDLayer) += (uClustersOfSensor * vClustersOfSensor);
  }



  if (m_eventCounter % m_PARAMstepSize == 0) {
    B2INFO("EventCounterModule - Event: " << m_eventCounter << " having " << aPxdClusterArray.getEntries() << "/" << aSvdClusterArray.getEntries() << " pxd/svdClusters. Detailed info: \n" << thisEventCounted.PrintStuff());
  } else {
    B2DEBUG(2, "EventCounterModule - Event: " << m_eventCounter << " having " << aPxdClusterArray.getEntries() << "/" << aSvdClusterArray.getEntries() << " pxd/svdClusters. Detailed info: \n" << thisEventCounted.PrintStuff())
  }

  m_allEventsCounted.push_back(thisEventCounted);
}



void EventCounterModule::endRun()
{
  if (m_eventCounter == 0) { m_eventCounter++; } // prevents division by zero
  double invEvents = 1. / m_eventCounter;
  B2INFO("EventCounterModule: after " << m_eventCounter << " events there were " << m_pxdClusterCounter << "/" << m_svdClusterCounter << " pxd/svdClusters total and " << double(m_pxdClusterCounter)*invEvents << "/" << double(m_svdClusterCounter)*invEvents << " pxd/svdClusters per event");


  vector<CountStuff<unsigned int>> logEventsCopy = m_allEventsCounted; // copying original since we want to change the internal order now for several times and do not want to break the original

  unsigned nQuantiles = 5;
  vector<CountStuff<unsigned int> > numberQuantiles(nQuantiles); // for each value we want to find the key figures, we store one entry. first is min, third is median, last is max
  vector<CountStuff<double> > occupancyQuantiles(nQuantiles); // for each value we want to find the key figures, we store one entry. first is min, third is median, last is max WARNING: xxxCluster-entries are to be ignored! they do not carry relevant data!



  // determining mean of results:
  CountStuff<unsigned int> collectedValues;
  for (auto & anEvent : logEventsCopy) {
    collectedValues += anEvent;
  }
  CountStuff<double> meanValues, meanOccupancy;
  meanValues =  CountStuff<double>(collectedValues); // (double) collectedValues; //
  meanValues *= invEvents;
  meanOccupancy = meanValues;
  meanOccupancy *= 100.;
  meanOccupancy = meanOccupancy / m_countedPixelsAndStrips;



  // providing numbers for relevant quantiles:
  int numLoggedEvents = logEventsCopy.size();
  int median = numLoggedEvents / 2;
  int q25 = numLoggedEvents / 4;
  int q75 = median + q25;



  // sort by PxdPixels:
  for (unsigned i = 0; i < c_nPXDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_pxdPixelCounter.at(i) < b.m_pxdPixelCounter.at(i);
    }
    );
    numberQuantiles.at(0).m_pxdPixelCounter.at(i) = logEventsCopy.at(0).m_pxdPixelCounter.at(i);
    numberQuantiles.at(1).m_pxdPixelCounter.at(i) = logEventsCopy.at(q25).m_pxdPixelCounter.at(i);
    numberQuantiles.at(2).m_pxdPixelCounter.at(i) = logEventsCopy.at(median).m_pxdPixelCounter.at(i);
    numberQuantiles.at(3).m_pxdPixelCounter.at(i) = logEventsCopy.at(q75).m_pxdPixelCounter.at(i);
    numberQuantiles.at(4).m_pxdPixelCounter.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_pxdPixelCounter.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_pxdPixelCounter) < b.sumUpArray(b.m_pxdPixelCounter); }
//     );
//   numberQuantiles.at(0).m_pxdPixelCounter = logEventsCopy.at(0).m_pxdPixelCounter;
//   numberQuantiles.at(1).m_pxdPixelCounter = logEventsCopy.at(q25).m_pxdPixelCounter;
//   numberQuantiles.at(2).m_pxdPixelCounter = logEventsCopy.at(median).m_pxdPixelCounter;
//   numberQuantiles.at(3).m_pxdPixelCounter = logEventsCopy.at(q75).m_pxdPixelCounter;
//   numberQuantiles.at(4).m_pxdPixelCounter = logEventsCopy.at(numLoggedEvents - 1).m_pxdPixelCounter;



  // sort by PxdClusters:
  for (unsigned i = 0; i < c_nPXDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_pxdClusterCounter.at(i) < b.m_pxdClusterCounter.at(i);
    }
    );
    numberQuantiles.at(0).m_pxdClusterCounter.at(i) = logEventsCopy.at(0).m_pxdClusterCounter.at(i);
    numberQuantiles.at(1).m_pxdClusterCounter.at(i) = logEventsCopy.at(q25).m_pxdClusterCounter.at(i);
    numberQuantiles.at(2).m_pxdClusterCounter.at(i) = logEventsCopy.at(median).m_pxdClusterCounter.at(i);
    numberQuantiles.at(3).m_pxdClusterCounter.at(i) = logEventsCopy.at(q75).m_pxdClusterCounter.at(i);
    numberQuantiles.at(4).m_pxdClusterCounter.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_pxdClusterCounter.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_pxdClusterCounter) < b.sumUpArray(b.m_pxdClusterCounter); }
//     );
//   numberQuantiles.at(0).m_pxdClusterCounter = logEventsCopy.at(0).m_pxdClusterCounter;
//   numberQuantiles.at(1).m_pxdClusterCounter = logEventsCopy.at(q25).m_pxdClusterCounter;
//   numberQuantiles.at(2).m_pxdClusterCounter = logEventsCopy.at(median).m_pxdClusterCounter;
//   numberQuantiles.at(3).m_pxdClusterCounter = logEventsCopy.at(q75).m_pxdClusterCounter;
//   numberQuantiles.at(4).m_pxdClusterCounter = logEventsCopy.at(numLoggedEvents - 1).m_pxdClusterCounter;



  // sort by SvdStripsU:
  for (unsigned i = 0; i < c_nSVDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_svdStripCounter4U.at(i) < b.m_svdStripCounter4U.at(i);
    }
    );
    numberQuantiles.at(0).m_svdStripCounter4U.at(i) = logEventsCopy.at(0).m_svdStripCounter4U.at(i);
    numberQuantiles.at(1).m_svdStripCounter4U.at(i) = logEventsCopy.at(q25).m_svdStripCounter4U.at(i);
    numberQuantiles.at(2).m_svdStripCounter4U.at(i) = logEventsCopy.at(median).m_svdStripCounter4U.at(i);
    numberQuantiles.at(3).m_svdStripCounter4U.at(i) = logEventsCopy.at(q75).m_svdStripCounter4U.at(i);
    numberQuantiles.at(4).m_svdStripCounter4U.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_svdStripCounter4U.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_svdStripCounter4U) < b.sumUpArray(b.m_svdStripCounter4U); }
//     );
//   numberQuantiles.at(0).m_svdStripCounter4U = logEventsCopy.at(0).m_svdStripCounter4U;
//   numberQuantiles.at(1).m_svdStripCounter4U = logEventsCopy.at(q25).m_svdStripCounter4U;
//   numberQuantiles.at(2).m_svdStripCounter4U = logEventsCopy.at(median).m_svdStripCounter4U;
//   numberQuantiles.at(3).m_svdStripCounter4U = logEventsCopy.at(q75).m_svdStripCounter4U;
//   numberQuantiles.at(4).m_svdStripCounter4U = logEventsCopy.at(numLoggedEvents - 1).m_svdStripCounter4U;



  // sort by SvdStripsV:
  for (unsigned i = 0; i < c_nSVDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_svdStripCounter4V.at(i) < b.m_svdStripCounter4V.at(i);
    }
    );
    numberQuantiles.at(0).m_svdStripCounter4V.at(i) = logEventsCopy.at(0).m_svdStripCounter4V.at(i);
    numberQuantiles.at(1).m_svdStripCounter4V.at(i) = logEventsCopy.at(q25).m_svdStripCounter4V.at(i);
    numberQuantiles.at(2).m_svdStripCounter4V.at(i) = logEventsCopy.at(median).m_svdStripCounter4V.at(i);
    numberQuantiles.at(3).m_svdStripCounter4V.at(i) = logEventsCopy.at(q75).m_svdStripCounter4V.at(i);
    numberQuantiles.at(4).m_svdStripCounter4V.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_svdStripCounter4V.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_svdStripCounter4V) < b.sumUpArray(b.m_svdStripCounter4V); }
//     );
//   numberQuantiles.at(0).m_svdStripCounter4V = logEventsCopy.at(0).m_svdStripCounter4V;
//   numberQuantiles.at(1).m_svdStripCounter4V = logEventsCopy.at(q25).m_svdStripCounter4V;
//   numberQuantiles.at(2).m_svdStripCounter4V = logEventsCopy.at(median).m_svdStripCounter4V;
//   numberQuantiles.at(3).m_svdStripCounter4V = logEventsCopy.at(q75).m_svdStripCounter4V;
//   numberQuantiles.at(4).m_svdStripCounter4V = logEventsCopy.at(numLoggedEvents - 1).m_svdStripCounter4V;



  // sort by SvdStripsTotal:
  for (unsigned i = 0; i < c_nSVDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_svdStripCounterTotal.at(i) < b.m_svdStripCounterTotal.at(i);
    }
    );
    numberQuantiles.at(0).m_svdStripCounterTotal.at(i) = logEventsCopy.at(0).m_svdStripCounterTotal.at(i);
    numberQuantiles.at(1).m_svdStripCounterTotal.at(i) = logEventsCopy.at(q25).m_svdStripCounterTotal.at(i);
    numberQuantiles.at(2).m_svdStripCounterTotal.at(i) = logEventsCopy.at(median).m_svdStripCounterTotal.at(i);
    numberQuantiles.at(3).m_svdStripCounterTotal.at(i) = logEventsCopy.at(q75).m_svdStripCounterTotal.at(i);
    numberQuantiles.at(4).m_svdStripCounterTotal.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_svdStripCounterTotal.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_svdStripCounterTotal) < b.sumUpArray(b.m_svdStripCounterTotal); }
//     );
//   numberQuantiles.at(0).m_svdStripCounterTotal = logEventsCopy.at(0).m_svdStripCounterTotal;
//   numberQuantiles.at(1).m_svdStripCounterTotal = logEventsCopy.at(q25).m_svdStripCounterTotal;
//   numberQuantiles.at(2).m_svdStripCounterTotal = logEventsCopy.at(median).m_svdStripCounterTotal;
//   numberQuantiles.at(3).m_svdStripCounterTotal = logEventsCopy.at(q75).m_svdStripCounterTotal;
//   numberQuantiles.at(4).m_svdStripCounterTotal = logEventsCopy.at(numLoggedEvents - 1).m_svdStripCounterTotal;


  // sort by SvdClusters:
  for (unsigned i = 0; i < c_nSVDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_svdClusterCounter.at(i) < b.m_svdClusterCounter.at(i);
    }
    );
    numberQuantiles.at(0).m_svdClusterCounter.at(i) = logEventsCopy.at(0).m_svdClusterCounter.at(i);
    numberQuantiles.at(1).m_svdClusterCounter.at(i) = logEventsCopy.at(q25).m_svdClusterCounter.at(i);
    numberQuantiles.at(2).m_svdClusterCounter.at(i) = logEventsCopy.at(median).m_svdClusterCounter.at(i);
    numberQuantiles.at(3).m_svdClusterCounter.at(i) = logEventsCopy.at(q75).m_svdClusterCounter.at(i);
    numberQuantiles.at(4).m_svdClusterCounter.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_svdClusterCounter.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_svdClusterCounter) < b.sumUpArray(b.m_svdClusterCounter); }
//     );
//   numberQuantiles.at(0).m_svdClusterCounter = logEventsCopy.at(0).m_svdClusterCounter;
//   numberQuantiles.at(1).m_svdClusterCounter = logEventsCopy.at(q25).m_svdClusterCounter;
//   numberQuantiles.at(2).m_svdClusterCounter = logEventsCopy.at(median).m_svdClusterCounter;
//   numberQuantiles.at(3).m_svdClusterCounter = logEventsCopy.at(q75).m_svdClusterCounter;
//   numberQuantiles.at(4).m_svdClusterCounter = logEventsCopy.at(numLoggedEvents - 1).m_svdClusterCounter;



  // sort by SvdClusterCombinations:
  for (unsigned i = 0; i < c_nSVDLayers; i++) {
    std::sort(
      logEventsCopy.begin(),
      logEventsCopy.end(),
    [i](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
      return a.m_svdClusterCombinationsCounter.at(i) < b.m_svdClusterCombinationsCounter.at(i);
    }
    );
    numberQuantiles.at(0).m_svdClusterCombinationsCounter.at(i) = logEventsCopy.at(0).m_svdClusterCombinationsCounter.at(i);
    numberQuantiles.at(1).m_svdClusterCombinationsCounter.at(i) = logEventsCopy.at(q25).m_svdClusterCombinationsCounter.at(i);
    numberQuantiles.at(2).m_svdClusterCombinationsCounter.at(i) = logEventsCopy.at(median).m_svdClusterCombinationsCounter.at(i);
    numberQuantiles.at(3).m_svdClusterCombinationsCounter.at(i) = logEventsCopy.at(q75).m_svdClusterCombinationsCounter.at(i);
    numberQuantiles.at(4).m_svdClusterCombinationsCounter.at(i) = logEventsCopy.at(numLoggedEvents - 1).m_svdClusterCombinationsCounter.at(i);
  }
//   std::sort(
//       logEventsCopy.begin(),
//       logEventsCopy.end(),
//       [](const CountStuff<unsigned int>& a, const CountStuff<unsigned int>& b) -> bool {
//        return a.sumUpArray(a.m_svdClusterCombinationsCounter) < b.sumUpArray(b.m_svdClusterCombinationsCounter); }
//     );
//   numberQuantiles.at(0).m_svdClusterCombinationsCounter = logEventsCopy.at(0).m_svdClusterCombinationsCounter;
//   numberQuantiles.at(1).m_svdClusterCombinationsCounter = logEventsCopy.at(q25).m_svdClusterCombinationsCounter;
//   numberQuantiles.at(2).m_svdClusterCombinationsCounter = logEventsCopy.at(median).m_svdClusterCombinationsCounter;
//   numberQuantiles.at(3).m_svdClusterCombinationsCounter = logEventsCopy.at(q75).m_svdClusterCombinationsCounter;
//   numberQuantiles.at(4).m_svdClusterCombinationsCounter = logEventsCopy.at(numLoggedEvents - 1).m_svdClusterCombinationsCounter;



  B2INFO("Detailed info about occured numbers: \n" <<
         " mean:\n" << meanValues.PrintStuff())
  B2DEBUG(1, " More detailed info about occured numbers: \n" <<
          " q0:\n" << numberQuantiles.at(0).PrintStuff() <<
          " q0.25:\n" << numberQuantiles.at(1).PrintStuff() <<
          " median:\n" << numberQuantiles.at(2).PrintStuff() <<
          " q0.75:\n" << numberQuantiles.at(3).PrintStuff() <<
          " q1:\n" << numberQuantiles.at(4).PrintStuff()
         )


  CountStuff<double> tempQuantile;
  for (unsigned int i = 0; i < nQuantiles; i++) {
    tempQuantile = CountStuff<double>(numberQuantiles.at(i));
    occupancyQuantiles.at(i) = tempQuantile / m_countedPixelsAndStrips;
    occupancyQuantiles.at(i) *= 100.;
  }

  B2INFO("\nDetailed info about occupancies in percent(%): \n" <<
         " mean:\n" << meanOccupancy.PrintStuff(false))
  B2DEBUG(1, "\nMore detailed info about occupancies in percent(%): \n" <<
          " q0:\n" << occupancyQuantiles.at(0).PrintStuff(false) <<
          " q0.25:\n" << occupancyQuantiles.at(1).PrintStuff(false) <<
          " median:\n" << occupancyQuantiles.at(2).PrintStuff(false) <<
          " q0.75:\n" << occupancyQuantiles.at(3).PrintStuff(false) <<
          " q1:\n" << occupancyQuantiles.at(4).PrintStuff(false)
         )
}


void EventCounterModule::InitializeCounters()
{
  m_eventCounter = 0;
  m_pxdClusterCounter = 0;
  m_svdStripCounter4U = 0;
  m_svdStripCounter4V = 0;
  m_svdClusterCounter = 0;
  m_svdClusterCombinationsCounter = 0;
}
