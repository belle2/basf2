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
#include <svd/geosvd/B2GeomSVDSensor.h>
#include <svd/geosvd/B2GeomOffset.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#else
#include "B2GeomSVDSensor.h"
#include "B2GeomOffset.h"
#endif

#include "TROOT.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TMath.h"
#include <vector>
#include <string>


#ifndef B2GEOMSVDLADDER_H_
#define B2GEOMSVDLADDER_H_

using namespace std;

#ifdef B2GEOM_BASF2
namespace Belle2 {
  class GearDir;
#endif

  class B2GeomSVDLadder {
  private:

    //! path of this Ladder
    string path;
#ifdef B2GEOM_BASF2
    GearDir ladderContent;
#endif

    // volumes in SVD ladder
    //! TGeoVolumeAssembly which contains all parts of this ladder
    TGeoVolumeAssembly* volSVDLadder;
    TGeoVolume* volRibsBarrel;
    TGeoVolume* volRibBarrel;
    TGeoVolume* volCarbonBarrel;
    TGeoVolume* volRibsSlanted;
    TGeoVolume* volRibSlanted;
    TGeoVolume* volCarbonSlanted;


    // Mediums used in SVD ladder
    TGeoMedium* medFoam;
    TGeoMedium* medCarbon;

    // Dimensions of the carbon+foam rib
    //! thickness of the ribs
    Double_t fThicknessRibs;
    //! position of the middle of each rib
    Double_t fRibUPosition0;
    Double_t fRibUPosition1;
    //! width of the ribs
    Double_t fWidthRibs;
    //! width of the carbon
    Double_t fWidthCarbon;
    //! length of the ribs in barrel region
    Double_t fLengthRibsBarrel;
    //! distance between middle of sensor and middle of carbon rib
    Double_t fRibsDistanceFromSensor;
    //! Length1 of the ribs in slanted region (theta angle => implementation of rib as Trd1)
    Double_t fLength1RibsSlanted;
    //! Length2 of the ribs in slanted region (theta angle => implementation of rib as Trd1)
    Double_t fLength2RibsSlanted;


    //! The sensor objects
    vector<B2GeomSVDSensor*> b2gSVDSensors;

    // Parameters
    //! layer number
    Int_t iLayer;
    //! ladder number
    Int_t iLadder;
    //! number of barrel sensors
    Int_t nSensors;

    //! The offsets of the sensors from their ideal position
    vector<B2GeomOffset*> b2gSensorOffsets;

    //! V Position of the sensors
    vector<Double_t> fSensorVPositions;

    //! W Position of the sensors
    vector<Double_t> fSensorWPositions;

    //! types of the sensors
    // 0 = small, 1 = normal, 2 = wedge
    vector<Int_t> sensorTypes;

    //! Angle of the sensors
    vector<Double_t> fThetas;

    //! Mediums contained in the ladder
    TGeoMedium* medAir;

    //! Methods to place components
    void putSensors();
    void putRibsBarrel();
    void putRibsSlanted();


  public:
    B2GeomSVDLadder();
    B2GeomSVDLadder(Int_t iLayer, Int_t iLadder);
    ~B2GeomSVDLadder();
#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
    Bool_t initOffsets();
#else
    Bool_t init();
#endif
    Bool_t make();
    TGeoVolumeAssembly* getVol() {
      return volSVDLadder;
    }
    TGeoHMatrix getOrigin();


  };
#ifdef B2GEOM_BASF2
}
#endif
#endif
