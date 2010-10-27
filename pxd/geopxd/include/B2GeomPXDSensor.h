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
#include <boost/format.hpp>
#include <pxd/geopxd/B2GeomVolume.h>

#include <string>
#include "TGeoMaterial.h"
#include "TROOT.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TGeoTrd2.h"
#include "TGeoCompositeShape.h"



#ifndef B2GEOMPXDSENSOR_H_
#define B2GEOMPXDSENSOR_H_

using namespace std;

namespace Belle2 {
  class GearDir;

  class B2GeomPXDSensorActive : public B2GeomVolume {
  private:
    string path;
    Int_t iLayer;
    Int_t iLadder;
    Int_t iSensor;
  public:
    B2GeomPXDSensorActive(Int_t iLay, Int_t iLad, Int_t iSen);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomPXDSensorThinned : public B2GeomVolume {
  private:
    string path;
    Int_t iLayer;
  public:
    B2GeomPXDSensorThinned(Int_t iLay);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomPXDSensorSilicon: public B2GeomVolume {
  private:
    string path;
    Int_t iLayer;
    Int_t iLadder;
    Int_t iSensor;
    B2GeomPXDSensorActive* volActive;
    B2GeomPXDSensorThinned* volThinned;
  public:
    B2GeomPXDSensorSilicon(Int_t iLay, Int_t iLad, Int_t iSen);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomPXDSensor : public B2GeomVolume {
  private:
    //! path of this Sensor
    string path;

    //! Volumes contained in the sensor
    B2GeomPXDSensorSilicon* volSilicon;

    // Parameters
    //! Layer number of this sensor
    Int_t iLayer;
    //! Ladder number of this sensor
    Int_t iLadder;
    //! Number of this sensor
    Int_t iSensor;

  public:
    B2GeomPXDSensor();
    B2GeomPXDSensor(Int_t iLayer , Int_t iLadder, Int_t iSensor);
    ~B2GeomPXDSensor();

    Bool_t init(GearDir& content);
    Bool_t make();
  };

}
#endif
