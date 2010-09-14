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
#include "TGeoTrd2.h"
#include "TGeoCompositeShape.h"



#ifndef B2GEOMPXDSENSOR_H_
#define B2GEOMPXDSENSOR_H_

using namespace std;

#ifdef B2GEOM_BASF2
namespace Belle2 {
  class GearDir;
#endif

  class B2GeomPXDSensor {
  private:
    //! path of this Sensor
    string path;

    //! TGeoVolumeAssembly which contains all parts of this sensor
    TGeoVolume* volPXDSensor;

    //! Volumes contained in the sensor
    TGeoVolume* volSilicon;
    TGeoVolume* volDEPFET;
    TGeoVolume* volActiveSensor;
    TGeoVolume* volSwitcher;
    TGeoVolume* volAir;

    //! Mediums contained in the sensor
    TGeoMedium* medAir;
    TGeoMedium* medDEPFET;
    TGeoMedium* medPXD_Silicon;

    // Parameters
    //! Layer number of this sensor
    Int_t iLayer;
    //! Ladder number of this sensor
    Int_t iLadder;
    //! Number of this sensor
    Int_t iSensor;

    //! Sensor type (1 = Laye1, 2 = Layer2, 9x = active sensor only)
    Int_t iSensorType;

    // box which contains whole sensor
    Double_t fSensorWidth;
    Double_t fSensorThick;
    Double_t fSensorLength;

    // box of active DEPFET area
    Double_t fLengthActive;
    Double_t fWidthActive;
    Double_t fThickActive;


    // distances of active area from borders of silicon
    Double_t fVDistanceActiveFromInnerEdge;
    Double_t fUDistanceActiveFromInnerEdge;

    // distances of thinned volume from borders of silicon
    Double_t fVDistanceThinnedFromOuterEdge;
    Double_t fVDistanceThinnedFromInnerEdge;
    Double_t fUDistanceThinnedFromOuterEdge;
    Double_t fUDistanceThinnedFromInnerEdge;

    // angle at the borders of thinned volume
    Double_t fAlphaThinned;

    // box of depfet silicon
    Double_t fLengthSilicon;
    Double_t fWidthSilicon;
    Double_t fThickSilicon;


    // Methods to place components
    void putDEPFET();
    void putSilicon();
    void putActiveSiliconOnly();
    void putSwitchers();

  public:
    B2GeomPXDSensor();
    B2GeomPXDSensor(Int_t iLayer , Int_t iLadder, Int_t iSensor);
    ~B2GeomPXDSensor();

#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
#else
    Bool_t init();
#endif
    Bool_t make();

    TGeoVolume* getVol() {
      /*
      volPXDSensor =  new TGeoVolumeAssembly("bla");
      char nameLayer[200];
      TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
      TGeoMedium* medLayer = new TGeoMedium("medLayer", 1, matVacuum);
      sprintf(nameLayer, "Layer_%i", iLayer);
      TGeoVolume* layer = gGeoManager->MakeBox(nameLayer, medLayer, 0.2, 0.2, 0.2);
      return layer;
      //volPXDSensor->AddNode(layer, 1, new TGeoTranslation(0,0,0));
      */
      return volPXDSensor;
    }
    Double_t getLengthSilicon();
    //! returns TGeoHMatrix which moves the sensor to the center of the surface
    TGeoHMatrix getSurfaceCenterPosition();
    //! returns TGeoHMatrix which points from the sensor(0,0,0) to the active_sensor(0,0,0)
    TGeoHMatrix getActiveSensorCenter();

  };

#ifdef B2GEOM_BASF2
}
#endif
#endif
