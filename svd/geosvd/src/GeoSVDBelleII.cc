/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/GeoSVDBelleII.h>

#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>

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

GeoSVDBelleII regGeoSVDBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

GeoSVDBelleII::GeoSVDBelleII() : CreatorBase("SVDBelleII")
{
  setDescription("Creates the TGeo objects for the SVD geometry of the Belle II detector.");
}


GeoSVDBelleII::~GeoSVDBelleII()
{

}


void GeoSVDBelleII::create(GearDir& content)
{
  //Get global parameters
  double globalRotAngle = content.getParamAngle("Rotation") / deg;
  double globalOffsetZ  = content.getParamLength("OffsetZ");
  string sensorMatName  = content.getParamString("MaterialSensor");

  TGeoVolume* topVolume = gGeoManager->GetTopVolume();
  TGeoVolumeAssembly* volGrpSVD = new TGeoVolumeAssembly("SVD");

  TGeoRotation* geoRot = new TGeoRotation("SVDRot", 90.0, globalRotAngle, 0.0);
  topVolume->AddNode(volGrpSVD, 1, new TGeoCombiTrans(0.0, 0.0, globalOffsetZ, geoRot)); //rotation followed by a translation

  //Get Material
  TGeoMedium* sensorMed = gGeoManager->GetMedium(sensorMatName.c_str());

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
    int    ladderShape  = int(layerContent.getParamNumValue("Shape"));
    double ladderLength = layerContent.getParamLength("Length");
    //double ladderWidth  = layerContent.getParamLength("Width");
    //double ladderWidth2 = layerContent.getParamLength("Width2");
    //double ladderThick  = layerContent.getParamLength("Thickness");
    int    nSensor      = int(layerContent.getParamNumValue("NumberOfSensors"));

    //Collect sensor data
    layerContent.append("Sensor/");
    double sensorLength      = layerContent.getParamLength("Length");
    double sensorWidth       = layerContent.getParamLength("Width");
    double sensorWidth2      = layerContent.getParamLength("Width2");
    double sensorGap         = layerContent.getParamLength("Gap");
    double sensorThick       = layerContent.getParamLength("Thickness");
    //double sensorPadSizeRPhi = layerContent.getParamLength("PadSizeRPhi");
    //double sensorPadSizeZ    = layerContent.getParamLength("PadSizeZ");

    double sensorRimWidthZ   = ((ladderLength - (nSensor - 1) * sensorGap) / nSensor - sensorLength) / 2.0; //rim around active sensor + active sensor length
    if (sensorRimWidthZ < (0.00001)) sensorRimWidthZ = 0; // Check if sensor rim not below precision (0.01 um)

    //Build geometry
    double ladderPhiRot = (2.0 * TMath::Pi()) / nLadder;
    TGeoVolumeAssembly* volGrpLadderGroup;
    TGeoVolumeAssembly* volGrpLadder;

    volGrpLadderGroup = new TGeoVolumeAssembly((format("SVDLadders_%1%") % iLayer).str().c_str());
    volGrpSVD->AddNode(volGrpLadderGroup, 1);

    for (int iLadder = 0; iLadder < nLadder; ++iLadder) {
      double currentPhi = layerPhi0 + ladderPhiRot * iLadder;

      volGrpLadder = new TGeoVolumeAssembly((format("SVDLadder_%1%_%2%") % iLayer % iLadder).str().c_str());
      TGeoRotation* ladderRot = new TGeoRotation((format("SVDLadderRot_%1%_%2%") % iLayer % iLadder).str().c_str());
      ladderRot->RotateX(layerTheta / deg);
      ladderRot->RotateZ((currentPhi / deg) - 90.0);
      TVector3 ladderPos(layerRadius, layerOffsetY, layerOffsetZ); // Ladder starting position
      ladderPos.RotateZ(currentPhi); // Ladder final position
      volGrpLadderGroup->AddNode(volGrpLadder, 1, new TGeoCombiTrans(ladderPos[0], ladderPos[1], ladderPos[2], ladderRot));

      for (int iSensor = 0; iSensor < nSensor; ++iSensor) {

        //--------------------
        //    Barrel part
        //--------------------
        if (ladderShape == 1) {

          //1) Build active sensors
          TGeoVolume* volSensorBox = gGeoManager->MakeBox((format("SVDSensor_%1%_%2%_%3%") % iLayer % iLadder % iSensor).str().c_str(),
                                                          sensorMed, sensorWidth * 0.5, sensorThick * 0.5, sensorLength * 0.5);
          volGrpLadder->AddNode(volSensorBox, 1, new TGeoTranslation(0.0, 0.0, -0.5*ladderLength + sensorRimWidthZ + 0.5*sensorLength +
                                                                     iSensor*(sensorLength + 2*sensorRimWidthZ + sensorGap)));
        }

        //--------------------
        //    Forward part
        //--------------------
        if (ladderShape == 2) {

          //1) Build active sensors
          TGeoVolume* volSensorTrd = gGeoManager->MakeTrd1((format("SVDSensor_%1%_%2%_%3%") % iLayer % iLadder % iSensor).str().c_str(),
                                                           sensorMed, sensorWidth * 0.5, sensorWidth2 * 0.5, sensorThick * 0.5, sensorLength * 0.5);
          volGrpLadder->AddNode(volSensorTrd, 1, new TGeoTranslation(0.0, 0.0, -0.5*ladderLength + sensorRimWidthZ + 0.5*sensorLength +
                                                                     iSensor*(sensorLength + 2*sensorRimWidthZ + sensorGap)));
        }
      }
    }
  }
}
