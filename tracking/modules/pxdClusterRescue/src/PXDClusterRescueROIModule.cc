/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Stefan Wunsch                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/pxdClusterRescue/PXDClusterRescueROIModule.h>

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <vxd/geometry/GeoCache.h>
#include <TMatrixDSym.h>

#include <math.h>

using namespace Belle2;

REG_MODULE(PXDClusterRescueROI)

PXDClusterRescueROIModule::PXDClusterRescueROIModule() : Module()
{
  // set description
  setDescription("Use tracks and find regions of interest (ROI) on PXD.");

  // set params
  addParam("gfTracksColName", m_gfTracksColName, "Name of genfit tracks in DataStore.", std::string("gfTracks"));
  addParam("ROIidColName", m_ROIidColName, "Name of ROIids in DataStore.", std::string(""));
  addParam("factorSigmaUV", m_factorSigmaUV,
           "Factor on uncertainties (sigmas of covariance entries) of U and V position after extrapolation.", float(5.0));
  addParam("tolerancePhi", m_tolerancePhi, "Tolerance by finding sensor in phi coordinate (radians).", float(0.15));
  addParam("toleranceZ", m_toleranceZ, "Tolerance by finding sensor in Z coordinate (cm).", float(0.5));
  addParam("maxROILengthU", m_maxROILengthU, "Maximum ROI length in U (pixels).", int(140));
  addParam("maxROILengthV", m_maxROILengthV, "Maximum ROI length in V (pixels).", int(140));
}

PXDClusterRescueROIModule::~PXDClusterRescueROIModule()
{
}

void PXDClusterRescueROIModule::initialize()
{
  // set needed data on required
  StoreArray<genfit::Track>::required(m_gfTracksColName);

  // register ROI objects
  StoreArray<ROIid>::registerPersistent(m_ROIidColName);
}

void PXDClusterRescueROIModule::beginRun()
{
}

void PXDClusterRescueROIModule::event()
{
  // load required data
  StoreArray<genfit::Track> gfTracks(m_gfTracksColName);
  StoreArray<ROIid> ROIids(m_ROIidColName);

  // get pxd geometry and iterators
  VXD::GeoCache& vxdGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> pxdLayers = vxdGeometry.getLayers(VXD::SensorInfoBase::PXD);
  std::set<Belle2::VxdID>::iterator pxdLayersIterator, pxdLaddersIterator, pxdSensorsIterator;

  // go through tracks
  genfit::MeasuredStateOnPlane gfTrackState; // current state of single track

  for (int k = 0; k < gfTracks.getEntries(); k++) {
    // get first pxd layer
    pxdLayersIterator = pxdLayers.begin();

    // extrapolate track to cylinders (PXD layers 1 and 2)
    for (unsigned int pxdLayer = 0; pxdLayer < pxdLayers.size(); pxdLayer++) {
      // get current state of track
      gfTrackState = gfTracks[k]->getFittedState();

      // do extrapolation to cylinder for given pxd layer radius
      try {
        gfTrackState.extrapolateToCylinder(m_pxdLayerRadius[pxdLayer]);
      } catch (genfit::Exception& gfException) {
        B2WARNING("Fitting failed: " << gfException.getExcString());
        continue; // jump to next layer or track
      }

      // get pxd ladders and iterator
      std::set<Belle2::VxdID> pxdLadders = vxdGeometry.getLadders(*pxdLayersIterator);
      pxdLaddersIterator = pxdLadders.begin();

      for (unsigned int pxdLadder = 0; pxdLadder < pxdLadders.size(); pxdLadder++) {
        // get pxd sensors and iterator
        std::set<Belle2::VxdID> pxdSensors = vxdGeometry.getSensors(*pxdLaddersIterator);
        pxdSensorsIterator = pxdSensors.begin();

        // go through sensors and check if extrapolation point is next to sensor
        // if true find ROI

        for (unsigned int pxdSensor = 0; pxdSensor < pxdSensors.size(); pxdSensor++) {
          VXD::SensorInfoBase pxdSensorInfo = vxdGeometry.getSensorInfo(*pxdSensorsIterator);

          // check whether sensor is in range of track
          bool isInRange = isSensorInRange(gfTrackState.getPos(), pxdSensorInfo);
          if (isInRange) {
            // get ROI on this sensor by extrapolating on it
            ROIid ROI;

            bool isROI = getROI(&ROI, gfTracks[k]->getFittedState(), pxdSensorInfo);

            // add ROI to DataStore if true
            if (isROI) {
              ROIids.appendNew(ROI);
            }
          }
          ++pxdSensorsIterator;
        }
        ++pxdLaddersIterator;
      }
      ++pxdLayersIterator;
    }
  }
}

bool PXDClusterRescueROIModule::getROI(ROIid* ROI, genfit::MeasuredStateOnPlane gfTrackState, VXD::SensorInfoBase& pxdSensorInfo)
{
  // extrapolate to sensor plane
  try {
    // get sensor plane
    TVector3 zeroVec(0, 0, 0);
    TVector3 uVec(1, 0, 0);
    TVector3 vVec(0, 1, 0);

    genfit::DetPlane* sensorPlane = new genfit::DetPlane();
    sensorPlane->setO(pxdSensorInfo.pointToGlobal(zeroVec));
    sensorPlane->setUV(pxdSensorInfo.vectorToGlobal(uVec), pxdSensorInfo.vectorToGlobal(vVec));

    genfit::SharedPlanePtr sensorPlaneSptr(sensorPlane);

    // do extrapolation
    gfTrackState.extrapolateToPlane(sensorPlaneSptr);
  } catch (genfit::Exception& gfException) {
    B2WARNING("Fitting failed: " << gfException.getExcString());
    return false;
  }

  // get intersection point in local coordinates with covariance matrix
  TVector3 gfTrackPosition = pxdSensorInfo.pointToLocal(gfTrackState.getPos()); // getPos returns cartesian in global system
  TMatrixDSym covMatrix = gfTrackState.getCov(); // 5D with elements q/p,u',v',u,v in plane system

  // get ROI by covariance matrix and local intersection point
  double widthU = std::sqrt(covMatrix(3, 3)) * m_factorSigmaUV;
  double widthV = std::sqrt(covMatrix(4, 4)) * m_factorSigmaUV;

  double widthUCellSize = pxdSensorInfo.getUSize() / pxdSensorInfo.getUCells();
  double widthVCellSize = pxdSensorInfo.getVSize() / pxdSensorInfo.getVCells();
  if (widthU * 2.0 / widthUCellSize > m_maxROILengthU) widthU = widthUCellSize * m_maxROILengthU /
        2.0; // resize ROI if estimation with covariance matrix is too large
  if (widthV * 2.0 / widthVCellSize > m_maxROILengthV) widthV = widthVCellSize * m_maxROILengthV / 2.0;

  double ROILocalU = gfTrackPosition.X();
  double ROILocalV = gfTrackPosition.Y();

  double uMin = ROILocalU - widthU;
  double uMax = ROILocalU + widthU;
  double vMin = ROILocalV - widthV;
  double vMax = ROILocalV + widthV;

  // set ROI
  ROI->setMinUid(pxdSensorInfo.getUCellID(uMin, 0, true)); // clamp=true ensures no outlayers
  ROI->setMaxUid(pxdSensorInfo.getUCellID(uMax, 0, true));
  ROI->setMinVid(pxdSensorInfo.getVCellID(vMin, true));
  ROI->setMaxVid(pxdSensorInfo.getVCellID(vMax, true));

  // set sensor id
  ROI->setSensorID(pxdSensorInfo.getID());

  return true;
}

bool PXDClusterRescueROIModule::isSensorInRange(TVector3 gfTrackPosition, VXD::SensorInfoBase& pxdSensorInfo)
{
  // get phi and z values of genfit track and sensor
  TVector3 zeroVec(0, 0, 0);
  TVector3 minVecV(0, -pxdSensorInfo.getVSize() / 2.0, 0);
  TVector3 minVecU(-pxdSensorInfo.getUSize() / 2.0, 0, 0);
  TVector3 maxVecV(0, pxdSensorInfo.getVSize() / 2.0, 0);
  TVector3 maxVecU(pxdSensorInfo.getUSize() / 2.0, 0, 0);

  double gfTrackPhi = std::atan2(gfTrackPosition.Y(), gfTrackPosition.X());
  double gfTrackZ = gfTrackPosition.Z();

  double sensorPhiMin = std::atan2(pxdSensorInfo.pointToGlobal(minVecU).Y(),
                                   pxdSensorInfo.pointToGlobal(minVecU).X()) - m_tolerancePhi;
  double sensorPhiMax = std::atan2(pxdSensorInfo.pointToGlobal(maxVecU).Y(),
                                   pxdSensorInfo.pointToGlobal(maxVecU).X()) + m_tolerancePhi;
  if (sensorPhiMin < -M_PI) sensorPhiMin = sensorPhiMin + 2.0 * M_PI; // correct 'underflows' of definition
  if (sensorPhiMax > M_PI) sensorPhiMax = sensorPhiMax - 2.0 * M_PI; // correct 'overflows' of definition

  double sensorZmin = pxdSensorInfo.pointToGlobal(minVecV).Z() - m_toleranceZ;
  double sensorZmax = pxdSensorInfo.pointToGlobal(maxVecV).Z() + m_toleranceZ;

  // check whether genfit track is in range
  if (not(sensorZmin <= gfTrackZ && sensorZmax >= gfTrackZ)) {
    return false;
  }
  // phi jumps from pi to -pi, in this case min>max and the processing has to differ
  if (sensorPhiMin < sensorPhiMax) { // standard case
    if (not(sensorPhiMin <= gfTrackPhi && sensorPhiMax >= gfTrackPhi)) {
      return false;
    }
  } else { // on pi->-pi
    if (not(sensorPhiMin <= gfTrackPhi && M_PI >= gfTrackPhi) && not(-M_PI <= gfTrackPhi && sensorPhiMax >= gfTrackPhi)) {
      return false;
    }
  }

  return true;
}

void PXDClusterRescueROIModule::endRun()
{
}

void PXDClusterRescueROIModule::terminate()
{
}
