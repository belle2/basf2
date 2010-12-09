/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMPXDSENSOR_H_
#define B2GEOMPXDSENSOR_H_

#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <framework/logging/Logger.h>
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


using namespace std;

namespace Belle2 {
  class GearDir;

  /** Class for building the active silicon of the PXD sensor. */
  class B2GeomPXDSensorActive : public B2GeomVXDVolume {
  public:
    /** Constructor for the active sensor */
    B2GeomPXDSensorActive() { }
    /** Reads parameters for PXD active silicon from GearBox. */
    Bool_t init(GearDir& content);
  };

  /** Class to build the silicon of the PXD Sensor. */
  class B2GeomPXDSensorSilicon: public B2GeomVXDVolume {
  private:
    B2GeomPXDSensorActive* volActive; /** < Object for building the active volume */
    B2GeomVXDVolume* volThinned; /** < Object for building the thinning. */
  public:
    /** Constructor for the Silicon of the PXD. */
    B2GeomPXDSensorSilicon();
    /** Reads Parameters for PXD Silicon from GearBox. */
    Bool_t init(GearDir& content);
    /** Builds the geometry of the PXD sensor silicon. */
    Bool_t make();

  };

  /** Class to build the PXD Sensor (silicon, active silicon, thinning, switchers). */
  class B2GeomPXDSensor : public B2GeomVXDStructVolume<B2GeomPXDSensor> {
  private:
    B2GeomPXDSensorSilicon* volSilicon; /** < Object for building the silicon including thinning and active volume. */
    B2GeomVXDVolume* volSwitchers1; /** < Object for building the switchers. */
    B2GeomVXDVolume* volSwitchers2; /** < Object for building the switchers. */

  public:
    /** Constructor for PXD Sensor. */
    B2GeomPXDSensor();
    /** Destructor for PXD Sensor. */
    ~B2GeomPXDSensor() { }
    /** Reads parameters from GearBox and creates objects for sub components. */
    Bool_t init(GearDir& content);
    /** Builds the PXD sensor geometry. */
    Bool_t make();
  };

}
#endif
