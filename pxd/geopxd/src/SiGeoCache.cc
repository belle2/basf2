/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/SiGeoCache.h>
#include <pxd/geopxd/CIDManager.h>

#include <framework/logging/Logger.h>

#include <TGeoManager.h>
#include <TGeoNode.h>

using namespace std;
using namespace Belle2;

const string activeSensorTag = "Silicon_Active";
const string pxdGeoTag = "PXD";
const string svdGeoTag = "SVD:not implemented.";
const string topGeoTag = "Top";

SiGeoCache* SiGeoCache::m_instance = NULL;

SiGeoCache* SiGeoCache::instance()
{
  if (!m_instance) {
    m_instance = new SiGeoCache();
  }
  return m_instance;
}

SiGeoCache::SiGeoCache() :
    m_iLaddersLayerID(-1), m_iSensorsLayerID(-1), m_iSensorsLadderID(-1),
    m_currentLayerID(-1), m_currentLadderID(-1), m_currentSensorID(-1),
    m_currentSensorUniID(-1), m_cellUniIDManager(new CellUniIDManager(0))
{
  refresh();
}

SiGeoCache::~SiGeoCache()
{
  m_sensors.clear();
  m_names.clear();
  if (m_cellUniIDManager != NULL)
    delete m_cellUniIDManager;
}

void SiGeoCache::refresh()
{
  // We don't clear maps, just overwrite existing entries.
  gGeoManager->CdTop();
  scanCurrentNode();

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

void SiGeoCache::scanCurrentNode()
{
  TGeoNode* node = gGeoManager->GetCurrentNode();
  string nodeName = node->GetName();
  if (nodeName.find(activeSensorTag) != string::npos) {
    // Make an entry into the SiSensorInfoMap
    SiSensorInfo newsensor(node);
    int newUniID = newsensor.getSensorUniID();
    m_sensors[newUniID] = newsensor;
    // Make an entry into the name map.
    string volName = node->GetVolume()->GetName();
    m_names[volName] = newUniID;
  }
  // Check daughters, but only if we are at the top or in a PXD or an SVD branch.
  if ((nodeName.find(pxdGeoTag) != string::npos)
      || (nodeName.find(svdGeoTag) != string::npos)
      || (nodeName.find(topGeoTag) != string::npos)
     )
    for (int iDaughter = 0; iDaughter < node->GetNdaughters(); iDaughter++) {
      gGeoManager->CdDown(iDaughter);
      scanCurrentNode();
    }
  gGeoManager->CdUp();
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
      if ((it->second.getLayerID() == iLayer) && (it->second.getLadderID()
                                                  == iLadder))
        m_iSensors.insert(it->second.getSensorID());
    }
    m_iSensorsLayerID = iLayer;
    m_iSensorsLadderID = iLadder;
  }
  return m_iSensors;
}

int SiGeoCache::getCellUniID(short int uCellID, short int vCellID)
{
  m_cellUniIDManager->setUCellID(uCellID);
  m_cellUniIDManager->setVCellID(vCellID);
  return m_cellUniIDManager->getCellUniID();
}

short int SiGeoCache::getUCellID(int cellUniID)
{
  m_cellUniIDManager->setCellUniID(cellUniID);
  return m_cellUniIDManager->getUCellID();
}

short int SiGeoCache::getVCellID(int cellUniID)
{
  m_cellUniIDManager->setCellUniID(cellUniID);
  return m_cellUniIDManager->getVCellID();
}

void SiGeoCache::getSensor(int iLayer, int iLadder, int iSensor)
{
  if (!isSameSensor(iLayer, iLadder, iSensor)) {
    // update current layer/ladder/sensor values
    m_currentLayerID = iLayer;
    m_currentLadderID = iLadder;
    m_currentSensorID = iSensor;
    // update UniID
    SensorUniIDManager uniIDMan(0);
    uniIDMan.setLayerID(iLayer);
    uniIDMan.setLadderID(iLadder);
    uniIDMan.setSensorID(iSensor);
    m_currentSensorUniID = uniIDMan.getSensorUniID();
    // get the desired sensor, invoke B2ERROR if not found.
    m_currentSensor = m_sensors[m_currentSensorUniID];
  }
  return;
}

void SiGeoCache::getSensor(int aSensorUniID)
{
  if (!isSameSensor(aSensorUniID)) {
    // update current layer/ladder/sensor values
    m_currentSensorUniID = aSensorUniID;
    SensorUniIDManager uniIDMan(aSensorUniID);
    m_currentLayerID = uniIDMan.getLayerID();
    m_currentLadderID = uniIDMan.getLadderID();
    m_currentSensorID = uniIDMan.getSensorID();
    m_currentSensor = m_sensors[m_currentSensorUniID];
  }
  return;
}


