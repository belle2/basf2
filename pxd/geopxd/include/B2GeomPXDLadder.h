/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomPXDSensor.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include <vector>
#include <string>


#ifndef B2GEOMPXDLADDER_H_
#define B2GEOMPXDLADDER_H_

using namespace std;
namespace Belle2 {

  class GearDir;

  class B2GeomPXDLadder {
  private:

    //! path of this Ladder
    string path;
    GearDir ladderContent;

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
    Bool_t init(GearDir& content);
    Bool_t make();
    TGeoVolumeAssembly* getVol() {
      return volPXDLadder;
    }
  };

}

#endif
