/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMSVDSENSOR_H_
#define B2GEOMSVDSENSOR_H_

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>
#include <pxd/geopxd/B2GeomVolume.h>
#include <string>
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TROOT.h"


using namespace std;

namespace Belle2 {

  class GearDir;

  class B2GeomSVDSensorSilicon : public B2GeomVXDVolume {
  private:
    B2GeomVXDVolume* volActive; /** < Object representing the geometry of the active sensor of SVD. */
  public:
    /** Constructor for geometry object representing the silicon of the SVD sensor. */
    B2GeomSVDSensorSilicon();
    /** Destructor for geometry object representing the silicon of the SVD sensor. */
    virtual ~B2GeomSVDSensorSilicon() {}
    /** Initialize Parameters from GearBox */
    Bool_t init(GearDir& content);
    /** Build TGeoVolume according to the parameters. */
    Bool_t make();
  };

  class B2GeomSVDSensor : public B2GeomVXDStructVolume<B2GeomSVDSensor> {
  private:
    B2GeomSVDSensorSilicon* volSilicon; /** < Object representing the geometry of the silicon of the SVD sensor. */
    B2GeomVXDVolume* volFoam; /** < Object representing the geometry of the insulating Rohacell Foam pad of the SVD sensor. */
    B2GeomVXDVolume* volSMDs; /** < Object representing the geometry of the chips on the SVD sensor */
    B2GeomVXDVolume* volKapton; /** < Object to represent the geometry of one type of kapton cable. */
    B2GeomVXDVolume* volKaptonFlex; /** < Object to represent the geometry of one type of kapton cable. */
    B2GeomVXDVolume* volKaptonFront; /** < Object to represent the geometry of one type of kapton cable. */

  public:
    /** Constructor for geometry object representing the SVD active sensor. */
    B2GeomSVDSensor();
    /** Destructor for geometry object representing the SVD active sensor. */
    virtual ~B2GeomSVDSensor() {}
    /** Initialize Parameters from GearBox */
    Bool_t init(GearDir& content);
    /** Build TGeoVolume according to the parameters. */
    Bool_t make();
  };
}
#endif
