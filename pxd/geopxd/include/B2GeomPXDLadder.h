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
#include <pxd/geopxd/B2GeomVolume.h>

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

namespace Belle2 {

  class GearDir;
  class B2GeomPXDLadder : public B2GeomVolume {
  private:

    //! path of this Ladder
    string path;

    //! Sensors of the ladder
    B2GeomPXDSensor** b2gPXDSensors;

    // Parameters
    //! layer number
    Int_t iLayer;
    //! ladder number
    Int_t iLadder;
    //! number of sensors
    Int_t nSensors;

  public:
    B2GeomPXDLadder();
    B2GeomPXDLadder(Int_t iLayer , Int_t iLadder);
    ~B2GeomPXDLadder();
    Bool_t init(GearDir& content);
    Bool_t make();
  };

}
#endif
