/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <pxd/geopxd/SiSensorInfo.h>

#include <pxd/dataobjects/PXDVolumeUserInfo.h>
#include <svd/dataobjects/SVDVolumeUserInfo.h>
#include <pxd/geopxd/CIDManager.h>

#include <string>

#include <TGeoManager.h>
#include <TGeoVolume.h>
#include "TGeoBBox.h"
#include "TGeoTrd2.h"

//#include <math>

using namespace std;
using namespace Belle2;

SiSensorInfo::SiSensorInfo(TGeoNode* pNode)
{
  // Initialize from the gGeoManager:
  // Get transformation matrix from the gGeoManager
  // Caution:
  // This uses TGeoNavigator! Can be dangerous to Geant4 tracking?!
  m_transform = dynamic_cast<TGeoHMatrix*>(gGeoManager->GetCurrentMatrix()->Clone());

  // To which detector this belongs
  TGeoVolume* pVolume = pNode->GetVolume();
  string volName = pVolume->GetName();
  if (volName.find("PXD") != string::npos)
    m_detectorType = c_PXD;
  else if (volName.find("SVD") != string::npos)
    m_detectorType = c_SVD;
  else
    m_detectorType = c_otherDetector;

  // Basic data from UserInfo
  if (m_detectorType == c_PXD) {
    PXDVolumeUserInfo* info =
      dynamic_cast<PXDVolumeUserInfo*>(pVolume->GetField());
    m_layerID = info->getLayerID();
    m_ladderID = info->getLadderID();
    m_sensorID = info->getSensorID();
    // Readout geometry data: read from UserInfo
    m_vPitch = info->getVPitch();
    m_uPitch = info->getUPitch();
    m_uPitchD = 0;
    m_vCells = info->getVCells();
    m_uCells = info->getUCells();
  } else {
    SVDVolumeUserInfo* info =
      dynamic_cast<SVDVolumeUserInfo*>(pVolume->GetField());
    m_layerID = info->getLayerID();
    m_ladderID = info->getLadderID();
    m_sensorID = info->getSensorID();
    // Readout geometry data from UserInfo
    /* Not implemented for SVD yet.
    m_vPitch = info->getUPitch();
    m_uPitch = info->getVPitch();
    m_uPitch2 = m_uPitch;
    m_vCells = info->getUCells();
    m_uCells = info->getVCells();
    */
  }

  SensorUniIDManager cid;
  cid.setLayerID(m_layerID);
  cid.setLadderID(m_ladderID);
  cid.setSensorID(m_sensorID);
  m_sensorUniID = cid.getSensorUniID();

  // Shape information
  TGeoShape* pShape = pVolume->GetShape();
  string classname(pShape->ClassName());

  if (classname == "TGeoTrd2") { // trapezoid - SVD
    m_shape = c_trapezoidal;
    TGeoTrd2 *shape = dynamic_cast<TGeoTrd2*>(pShape);
    m_thickness = 2.0 * shape->GetDx1();
    double uSize = 2.0 * shape->GetDy1();
    double uSize2 = 2.0 * shape->GetDy2();
    m_uSize = 0.5 * (uSize + uSize2);
    m_vSize = 2.0 * shape->GetDz();
    m_uSizeD = 2.0 * (uSize2 - uSize) / m_vSize;
    // Consolidate: adapt cell size to pitch and number of cells.
    // To be removed once geometry is consistent.
    //m_uSize = m_uPitch * m_uCells;
    //m_vSize = m_vPitch * m_vCells;

  } else if (classname == "TGeoBBox") { // bounding box - PXD
    m_shape = c_rectangular;
    TGeoBBox *shape = dynamic_cast<TGeoBBox*>(pShape);
    m_thickness = 2.0 * shape->GetDX();
    m_uSize = 2.0 * shape->GetDY();
    m_uSizeD = 0;
    m_vSize = 2.0 * shape->GetDZ();

  } else {
    B2ERROR("Unknown shape of active Si sensor detected: " << classname.c_str());
    m_shape = c_otherShape;
    m_uSize = 1.0;
    m_uSizeD = 0.0;
    m_vSize = 1.0;
  } // if classname

} // constructor

SiSensorInfo::~SiSensorInfo() {;}

// Cell IDs to coordinates and v.v.

int SiSensorInfo::getVCellID(double v) const
{
  return (static_cast<int>((0.5 * m_vSize + v) / m_vPitch));
}


int SiSensorInfo::getUCellID(double u, double v) const
{
  double uSize = getUSize(v);
  double uPitch = getUPitch(v);
  return (static_cast<int>((0.5 * uSize + u) / uPitch));
}

double SiSensorInfo::getVCellPosition(int vID) const
{
  return (vID + 0.5) * m_vPitch - 0.5 * m_vSize;
}

double SiSensorInfo::getUCellPosition(int uID, int vID) const
{
  double v = 0;
  if (vID > -1) v = getVCellPosition(vID);
  return (uID + 0.5) * getUPitch(v) - 0.5 * getUSize(v);
}


// Coordinate transforms.

void SiSensorInfo::localToMaster(const TVector3& local, TVector3& master) const
{
  double clocal[3];
  double cmaster[3];
  local.GetXYZ(clocal);
  m_transform->LocalToMaster(clocal, cmaster);
  master.SetXYZ(cmaster[0], cmaster[1], cmaster[2]);
}


void SiSensorInfo::localToMasterVec(const TVector3& local, TVector3& master) const
{
  double clocal[3];
  double cmaster[3];
  local.GetXYZ(clocal);
  m_transform->LocalToMasterVect(clocal, cmaster);
  master.SetXYZ(cmaster[0], cmaster[1], cmaster[2]);
}


void SiSensorInfo::masterToLocal(const TVector3& master, TVector3& local) const
{
  double clocal[3];
  double cmaster[3];
  master.GetXYZ(cmaster);
  m_transform->MasterToLocal(cmaster, clocal);
  local.SetXYZ(clocal[0], clocal[1], clocal[2]);
}


void SiSensorInfo::masterToLocalVec(const TVector3& master, TVector3& local) const
{
  double clocal[3];
  double cmaster[3];
  master.GetXYZ(cmaster);
  m_transform->MasterToLocalVect(cmaster, clocal);
  local.SetXYZ(clocal[0], clocal[1], clocal[2]);
}


