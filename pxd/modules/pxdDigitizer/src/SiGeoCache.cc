/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <pxd/modules/pxdDigitizer/SiGeoCache.h>
#include <pxd/hitpxd/CIDManager.h>

#include <TGeoManager.h>
#include <TGeoNode.h>

using namespace std;
using namespace Belle2;

const string activeSensorTag = "Silicon_Active";
const string pxdGeoTag = "PXD";
const string svdGeoTag = "SVD";

SiGeoCache::SiGeoCache(TGeoManager* aGeoManager):
    m_iLaddersLayerID(-1), m_iSensorsLayerID(-1), m_iSensorsLadderID(-1),
    m_currentLayerID(-1), m_currentLadderID(-1), m_currentSensorID(-1), m_currentCID(-1)
{
  Initialize(aGeoManager);
}

SiGeoCache::~SiGeoCache()
{
  m_sensors.clear();
  m_names.clear();
}

void SiGeoCache::Initialize(TGeoManager* aGeoManager)
{
  // We don't clear maps, just overwrite existing entries.
  aGeoManager->CdTop();
  scanNode(aGeoManager);

  // Initialize sets of PXD and SVD layer indices.
  for (SiSensorInfoItr it = m_sensors.begin(); it != m_sensors.end(); ++it) {
    switch (it->second.getDetectorType()) {
      case SiSensorInfo::c_PXD:
        m_iLayersPXD.insert(it->second.getLayerID());
        break;
      case SiSensorInfo::c_SVD:
        m_iLayersSVD.insert(it->second.getLayerID());
        break;
      default:
        ;
    } // switch
  } // for
}

void SiGeoCache::scanNode(TGeoManager *aGeoManager)
{
  TGeoNode* node = aGeoManager->GetCurrentNode();
  string nodeName = node->GetName();
  if (nodeName.find(activeSensorTag) != string::npos) {
    // Make an entry into the SiSensorInfoMap
    SiSensorInfo newsensor(node);
    int newCID = newsensor.getCID();
    m_sensors[newCID] = newsensor;
    // Make an entry into the name map.
    string volName = node->GetVolume()->GetName();
    m_names[volName] = newCID;
  }
  // Check daughters, but only if we are in a PXD or an SVD branch.
  if ((nodeName.find(pxdGeoTag) != string::npos) || (nodeName.find(svdGeoTag) != string::npos))
    for (int iDaughter = 0; iDaughter < node->GetNdaughters(); iDaughter++) {
      aGeoManager->CdDown(iDaughter);
      scanNode(aGeoManager);
    }
  aGeoManager->CdUp();
  return;
}

/* Basic layout getters.*/

SiGeoCache::SiLayerType SiGeoCache::getLayerType(int iLayer) const
{
  if (m_iLayersPXD.count(iLayer) == 1)
    return c_pixel;
  else if (m_iLayersSVD.count(iLayer) == 1)
    return c_strip;
  else
    return c_otherLayerType;
}

const set<int>& SiGeoCache::getLadderIDs(int iLayer)
{
  if (iLayer != m_iLaddersLayerID) {
    // re-calculate
    m_iLadders.clear();
    for (SiSensorInfoItr it = m_sensors.begin(); it != m_sensors.end(); ++it) {
      if (it->second.getLayerID() == iLayer)
        m_iLadders.insert(it->second.getLadderID());
    }
    m_iLaddersLayerID = iLayer;
  }
  return m_iLadders;
}

const set<int>& SiGeoCache::getSensorIDs(int iLayer, int iLadder)
{
  if ((iLayer != m_iSensorsLayerID) || (iLadder != m_iSensorsLadderID)) {
    // recalculate
    m_iSensors.clear();
    for (SiSensorInfoItr it = m_sensors.begin(); it != m_sensors.end(); ++it) {
      if ((it->second.getLayerID() == iLayer) && (it->second.getLadderID() == iLadder))
        m_iSensors.insert(it->second.getSensorID());
    }
    m_iSensorsLayerID = iLayer;
    m_iSensorsLadderID = iLadder;
  }
  return m_iSensors;
}


void SiGeoCache::getSensor(int iLayer, int iLadder, int iSensor)
{
  if (!isSameSensor(iLayer, iLadder, iSensor)) {
    // update current layer/ladder/sensor values
    m_currentLayerID = iLayer;
    m_currentLadderID = iLadder;
    m_currentSensorID = iSensor;
    // update CID
    CIDManager cidMan(0);
    cidMan.setLayerID(iLayer);
    cidMan.setLadderID(iLadder);
    cidMan.setSensorID(iSensor);
    m_currentCID = cidMan.getCID();
    // get the desired sensor, invoke B2ERROR if not found.
    m_currentSensor = m_sensors[m_currentCID];
  }
}

double SiGeoCache::getVSensorSize(int iLayer, int iLadder, int iSensor, double u)
{
  getSensor(iLayer, iLadder, iSensor);
  switch (m_currentSensor.getShape()) {
    case SiSensorInfo::c_rectangular:
      return m_currentSensor.getVSize();
    case SiSensorInfo::c_trapezoidal:
      return m_currentSensor.getVSize(u);
    default:
      return 0;
  } // switch
}

double SiGeoCache::getVSensorPitch(int iLayer, int iLadder, int iSensor, double u)
{
  getSensor(iLayer, iLadder, iSensor);
  switch (m_currentSensor.getShape()) {
    case SiSensorInfo::c_rectangular:
      return m_currentSensor.getVPitch();
    case SiSensorInfo::c_trapezoidal:
      return m_currentSensor.getVPitch(u);
    default:
      return 0;
  } // switch
}


