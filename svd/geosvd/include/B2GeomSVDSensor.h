/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef B2GEOM_BASF2
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#endif

#include <string>
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TROOT.h"



#ifndef B2GEOMSVDSENSOR_H_
#define B2GEOMSVDSENSOR_H_

using namespace std;

#ifdef B2GEOM_BASF2
namespace Belle2 {

  class GearDir;
#endif

  class B2GeomSVDSensor {
  private:

    //! path of this Sensor
    string path;

    //! TGeoVolumeAssembly which contains all parts of this sensor
    TGeoVolumeAssembly* volSVDSensor;

    //! Volumes contained in the sensor
    TGeoVolume* volActiveSensor;
    TGeoVolume* volSilicon;
    TGeoVolume* volFoam;
    TGeoVolume* volAir;
    TGeoVolume* volKapton;
    TGeoVolume* volCoolPipe;
    TGeoVolume* volSMDs;

    //! Mediums contained in the sensor
    TGeoMedium* medAir;
    TGeoMedium* medSVD_Silicon;
    TGeoMedium* medSVD_Foam;
    TGeoMedium* medSVD_Kapton;
    TGeoMedium* medSVD_CoolPipeSteel;
    TGeoMedium* medSVD_SMDs;

    // Parameters
    //! Layer number of this sensor
    Int_t iLayer;
    //! Ladder number of this sensor
    Int_t iLadder;
    //! Number of this sensor
    Int_t iSensor;
    //! Sensor type (0 = small, 1 = normal, 2 = wedge)
    Int_t iSensorType;

    //! Dimensions of a box which contains the whole sensor
    Double_t fSensorLength;
    Double_t fSensorWidth;
    Double_t fSensorWidth2;
    Double_t fSensorThick;

    //! Dimensions of a box which contains the silicon (active + passive)
    Double_t fSiliconLength;
    Double_t fSiliconWidth;
    Double_t fSiliconWidth2;
    Double_t fSiliconThick;

    //! Dimensions of the active part of the sensor
    Double_t fActiveSensorLength;
    Double_t fActiveSensorWidth;
    Double_t fActiveSensorWidth2;
    Double_t fActiveSensorThick;

    //! Thickness of the foam
    Double_t fThickFoam;

    //! Thickness of the kapton
    Double_t fThickKapton;

    //! Outer radius of the cooling pipe
    Double_t fOuterRadiusCoolPipe;
    //! Inner radius of the cooling pipe
    Double_t fInnerRadiusCoolPipe;
    //! Position of the cooling pipe in U direction
    Double_t fUPositionCoolPipe;

    //! Has this sensor SMDs?
    Bool_t isSMDs;
    //! Width of the SMDs
    Double_t fWidthSMDs;
    //! Thickness of the SMDs
    Double_t fThickSMDs;
    //! Position of the SMDs in U direction
    Double_t fUPositionSMDs;

    //! Methods to place components
    void putSilicon();
    void putFoam();
    void putKapton();
    void putCoolingPipe();
    void putSMDs();

  public:

    B2GeomSVDSensor();
    B2GeomSVDSensor(Int_t iLay, Int_t iLad, Int_t iSen, Int_t iST);
    ~B2GeomSVDSensor();
#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
#else
    Bool_t init();
#endif
    Bool_t make();
    TGeoVolumeAssembly* getVol() {
      return volSVDSensor;
    }
    Double_t getLength() {
      return fSensorLength;
    }
    Double_t getWidth() {
      return fSensorWidth;
    }
    //! returns the Coordinate of the center of the inner surface of the silicon
    TGeoHMatrix getSurfaceCenterPosition();


  };
#ifdef B2GEOM_BASF2
}
#endif
#endif
