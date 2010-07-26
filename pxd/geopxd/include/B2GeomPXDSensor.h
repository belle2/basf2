/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <string>
#include <boost/format.hpp>
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"



#ifndef B2GEOMPXDSENSOR_H_
#define B2GEOMPXDSENSOR_H_

using namespace std;

namespace Belle2 {
  class GearDir;

  class B2GeomPXDSensor {
  private:
    //! path of this Sensor
    string path;

    //! TGeoVolumeAssembly which contains all parts of this sensor
    TGeoVolumeAssembly* volPXDSensor;

    //! Volumes contained in the sensor
    TGeoVolume* volDEPFET;
    TGeoVolume* volActiveSensor;
    TGeoVolume* volSwitcher;
    TGeoVolume* volAir;

    //! Mediums contained in the sensor
    TGeoMedium* medAir;
    TGeoMedium* medDEPFET;
    TGeoMedium* medActiveSensor;

    // Parameters
    //! Layer number of this sensor
    Int_t iLayer;
    //! Ladder number of this sensor
    Int_t iLadder;
    //! Number of this sensor
    Int_t iSensor;
    Double_t fSensorLength;
    Double_t fSensorWidth;
    Double_t fSensorThick;
    Double_t fActiveSensorLength;
    Double_t fActiveSensorWidth;
    Double_t fActiveSensorThick;


    //! Methods to place components
    void putDEPFET();
    void putSwitchers();

  public:
    B2GeomPXDSensor();
    B2GeomPXDSensor(Int_t iLayer , Int_t iLadder, Int_t iSensor);
    ~B2GeomPXDSensor();

    Bool_t init(GearDir& content);
    Bool_t init();
    Bool_t make();
    TGeoVolumeAssembly* getVol() {
      return volPXDSensor;
    }
    Double_t getLength() {
      return fSensorLength;
    }

  };
}

#endif
