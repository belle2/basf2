/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geopxd/GeoPXDBelleII.h>

#include <gearbox/GearDir.h>
#include <datastore/Units.h>

#include <cmath>
#include <boost/format.hpp>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>


using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoPXDBelleII regGeoPXDBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoPXDBelleII::GeoPXDBelleII() : CreatorBase("PXDBelleII")
{
  setDescription("Creates the TGeo objects for the PXD geometry of the Belle II detector.");
  activateAutoSensitiveVolumes("SD_"); //The PXD subdetector uses the "SD_" prefix to flag its sensitive volumes
}


GeoPXDBelleII::~GeoPXDBelleII()
{

}


void GeoPXDBelleII::create(GearDir& content)
{
  //----------------------------------------
  //         Get global parameters
  //----------------------------------------
  double globalRotAngle = content.getParamAngle("Rotation") / deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");
  string sensorMatName  = content.getParamString("MaterialSensor");


  //----------------------------------------
  //        Add subdetector group
  //----------------------------------------
  TGeoRotation* geoRot = new TGeoRotation("PXDRot", 90.0, globalRotAngle, 0.0);
  TGeoVolumeAssembly* volGrpPXD = addSubdetectorGroup("PXD", new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot));


  //----------------------------------------
  //            Get Material
  //----------------------------------------
  TGeoMedium* sensorMed = gGeoManager->GetMedium(sensorMatName.c_str());


  //----------------------------------------
  //           Build subdetector
  //----------------------------------------

  //Get number of layers
  int nLayer = content.getNumberNodes("Layers/Layer");

  for (int iLayer = 1; iLayer <= nLayer; ++iLayer) {

    GearDir layerContent(content);
    layerContent.append((format("Layers/Layer[%1%]/") % (iLayer)).str());

    //Collect layer data
    double layerPhi0    = layerContent.getParamAngle("Phi0");
    double layerTheta   = layerContent.getParamAngle("Theta");
    double layerRadius  = layerContent.getParamLength("Radius");
    double layerOffsetY = layerContent.getParamLength("OffsetY");
    double layerOffsetZ = layerContent.getParamLength("OffsetZ");
    int    nLadder      = int(layerContent.getParamNumValue("NumberOfLadders"));

    //Collect ladder data
    layerContent.append("Ladder/");
    double ladderLength     = layerContent.getParamLength("Length");
    //double ladderWidth      = layerContent.getParamLength("Width");
    //double ladderThick      = layerContent.getParamLength("Thickness");
    //double switcherDistance = layerContent.getParamLength("PXLSwitcherDistance");
    int    nSensor          = int(layerContent.getParamNumValue("NumberOfSensors"));

    //Collect sensor data
    layerContent.append("Sensor/");
    double sensorLength      = layerContent.getParamLength("Length");
    double sensorWidth       = layerContent.getParamLength("Width");
    double sensorGap         = layerContent.getParamLength("Gap");
    double sensorThick       = layerContent.getParamLength("Thickness");
    //double sensorPadSizeRPhi = layerContent.getParamLength("PadSizeRPhi");
    //double sensorPadSizeZ    = layerContent.getParamLength("PadSizeZ");

    double sensorRimWidthZ   = ((ladderLength - (nSensor - 1) * sensorGap) / nSensor - sensorLength) / 2.0; //rim around active sensor + active sensor length
    if (sensorRimWidthZ < (0.00001)) sensorRimWidthZ = 0; // Check if sensor rim not below precision (0.01 um)

    //Build geometry
    double ladderPhiRot = (2.0 * M_PI) / nLadder;
    TGeoVolumeAssembly* volGrpLayer;
    TGeoVolumeAssembly* volGrpLadder;

    volGrpLayer = new TGeoVolumeAssembly((format("PXDLayer_%1%") % iLayer).str().c_str());
    volGrpPXD->AddNode(volGrpLayer, 1);

    for (int iLadder = 0; iLadder < nLadder; ++iLadder) {
      double currentPhi = layerPhi0 + ladderPhiRot * iLadder;

      volGrpLadder = new TGeoVolumeAssembly((format("PXDLadder_%1%_%2%") % iLayer % iLadder).str().c_str());
      TGeoRotation* ladderRot = new TGeoRotation((format("PXDLadderRot_%1%_%2%") % iLayer % iLadder).str().c_str());
      ladderRot->RotateX(layerTheta / deg);
      ladderRot->RotateZ((currentPhi / deg) - 90.0);
      TVector3 ladderPos(layerRadius, layerOffsetY, layerOffsetZ); // Ladder starting position
      ladderPos.RotateZ(currentPhi); // Ladder final position
      volGrpLayer->AddNode(volGrpLadder, 1, new TGeoCombiTrans(ladderPos[0], ladderPos[1], ladderPos[2], ladderRot));

      for (int iSensor = 0; iSensor < nSensor; ++iSensor) {

        //1) Build active sensors (The SD prefix (Sensitive Detector), flags the volume to be a sensitive detector)
        TGeoVolume* volSensorBox = gGeoManager->MakeBox((format("SD_PXDSensor_%1%_%2%_%3%") % iLayer % iLadder % iSensor).str().c_str(),
                                                        sensorMed, sensorWidth * 0.5, sensorThick * 0.5, sensorLength * 0.5);

        volGrpLadder->AddNode(volSensorBox, 1, new TGeoTranslation(0.0, 0.0, -0.5*ladderLength + 0.5*(sensorLength + 2*sensorRimWidthZ) +
                                                                   iSensor*(sensorLength + 2*sensorRimWidthZ + sensorGap)));
      }
    }
  }
}
