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
#include <pxd/geopxd/B2GeomPXDSensor.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#else
#include "B2GeomPXDSensor.h"
#endif

#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include <string>
#include <vector>


#ifndef B2GEOMPXDLADDER_H_
#define B2GEOMPXDLADDER_H_

using namespace std;

#ifdef B2GEOM_BASF2
namespace Belle2 {

  class GearDir;
#endif
  class B2GeomPXDLadder {
  private:

    //! path of this Ladder
    string path;
#ifdef B2GEOM_BASF2
    GearDir ladderContent;
#endif

    //! TGeoVolumeAssembly which contains all parts of this sensor
    TGeoVolumeAssembly* volPXDLadder;

    //! Volumes contained in the sensor
    vector<B2GeomPXDSensor*> b2gPXDSensors;

    // Parameters
    //! layer number
    Int_t iLayer;
    //! ladder number
    Int_t iLadder;
    //! number of sensors
    Int_t nSensors;
    Double_t fSensorLength;
    Double_t fSensorWidth;
    Double_t fSensorThick;
    Double_t fGapLength;

    //! Mediums contained in the sensor
    TGeoMedium* medAir;
    TGeoMedium* medGlue;


    //! Methods to place components
    void putSensors();
    void putGlue();

  public:
    B2GeomPXDLadder();
    B2GeomPXDLadder(Int_t iLayer , Int_t iLadder);
    ~B2GeomPXDLadder();
#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
#else
    Bool_t init();
#endif
    Bool_t make();
    TGeoVolumeAssembly* getVol() {
      return volPXDLadder;
    }
  };
#ifdef B2GEOM_BASF2
}
#endif

#endif
