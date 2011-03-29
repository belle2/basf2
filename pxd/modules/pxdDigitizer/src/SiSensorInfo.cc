/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <pxd/modules/pxdDigitizer/SiSensorInfo.h>

#include <pxd/dataobjects/PXDVolumeUserInfo.h>
#include <pxd/hitpxd/CIDManager.h>

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

  // Get transformation matrix from the node.
  m_transform = pNode->GetMatrix();

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
  PXDVolumeUserInfo* info = dynamic_cast<PXDVolumeUserInfo*>(pVolume->GetField());
  m_layerID = info->getLayerID();
  m_ladderID = info->getLadderID();
  m_sensorID = info->getSensorID();
  CIDManager cid;
  cid.setLayerID(m_layerID);
  cid.setLadderID(m_ladderID);
  cid.setSensorID(m_sensorID);
  m_CID = cid.getCID();

  // Readout geometry data from UserInfo
  m_uPitch = info->getUPitch();
  m_vPitch = info->getVPitch();
  m_vPitch2 = m_vPitch; //info->getVPitch2();
  m_uCells = info->getUCells();
  m_vCells = info->getVCells();

  // Shape information
  TGeoShape* pShape = pVolume->GetShape();
  string classname(pShape->ClassName());

  if (classname == "TGeoTrd2") {   // trapezoid - SVD
    m_shape = c_trapezoidal;
    TGeoTrd2 *shape = dynamic_cast<TGeoTrd2*>(pShape);
    m_thickness = 2.0 * shape->GetDx1();
    m_vSize = 2.0 * shape->GetDy1();
    m_vSize2 = 2.0 * shape->GetDy2();
    m_uSize = 2.0 * shape->GetDz();

  } else if (classname == "TGeoBBox") { // bounding box - PXD
    m_shape = c_rectangular;
    TGeoBBox *shape = dynamic_cast<TGeoBBox*>(pShape);
    m_thickness = 2.0 * shape->GetDX();
    m_vSize = 2.0 * shape->GetDY();
    m_vSize2 = m_vSize;
    m_uSize = 2.0 * shape->GetDZ();

  } else {
    B2ERROR("Unknown shape of active Si sensor detected: " << classname.c_str());
    m_shape = c_otherShape;
    return;
  } // if classname

} // constructor

SiSensorInfo::~SiSensorInfo() {;}

// Cell IDs to coordinates and v.v.

int SiSensorInfo::getUCellID(double u) const
{
  if (fabs(u) > 0.5*m_uSize) return -1;
  else {
    return static_cast<int>((0.5*m_uSize + u) / m_uPitch);
  }
}


int SiSensorInfo::getVCellID(double v) const
{
  if (m_shape != c_rectangular) {
    B2ERROR("Incorrect SiSensorInfo method for non-rectangular detector !!!")
    return -1;
  }
  if (fabs(v) > 0.5*m_vSize) return -1;
  else {
    return static_cast<int>((0.5*m_vSize + v) / m_vPitch);
  }
}


int SiSensorInfo::getVCellID(double u, double v) const
{
  double vSize = getVSize(u);
  if (fabs(v) > 0.5*vSize) return -1;
  else {
    double vPitch = getVPitch(u);
    return static_cast<int>((vSize + v) / vPitch);
  }
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


