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

    //! TGeoVolumeAssembly which contains all parts of this ladder
    TGeoVolumeAssembly* volSVDLadder;



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
    //! Gives the Length of the barrel style part of the SVD ladder (sum over sensor lengths plus gaps)
    Double_t getLengthBarrel();

  };
#ifdef B2GEOM_BASF2
}
#endif
#endif
