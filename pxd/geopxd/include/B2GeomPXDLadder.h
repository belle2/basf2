/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMPXDLADDER_H_
#define B2GEOMPXDLADDER_H_

#include <pxd/geopxd/B2GeomPXDSensor.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>
#include <pxd/geopxd/B2GeomVolume.h>

#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include <string>
#include <vector>

using namespace std;

namespace Belle2 {

  class GearDir;
  class B2GeomPXDLadder : public B2GeomVXDStructVolume<B2GeomPXDSensor> {
  public:
    /** Constructor for a PXD ladder geometry object. */
    B2GeomPXDLadder() { }
    /** init PXD ladder geometry object with parameters from GearDir. */
    Bool_t init(GearDir& content);
    /** create TGeoVolume from the parameters. */
    Bool_t make();
  };

}
#endif
