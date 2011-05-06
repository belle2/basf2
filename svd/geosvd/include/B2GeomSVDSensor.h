/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMSVDSENSOR_H
#define B2GEOMSVDSENSOR_H

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

  /** Class for building the active silicon of the SVD sensor. */
  class B2GeomSVDSensorActive : public B2GeomVXDVolume {
  public:
    /** Constructor for the active sensor */
    B2GeomSVDSensorActive() { }
    /** Destructor for the active sensor */
    virtual ~B2GeomSVDSensorActive() { }
    /** Reads parameters for PXD active silicon (inclusive UserInfo) from GearBox.
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    Bool_t init(GearDir& content);
    /** Build the geometry and adds PXDUserInfo for the active silicon. */
    Bool_t make();
  private:
    double m_uPitch;      /** < detector pitch in U direction. */
    double m_uPitch2;       /** < detector pitch in U direction for wedge sensor. */
    int m_uCells;       /** < number of cells in U direction. */
    double m_vPitch;      /** < detector pitch in V direction. */
    int m_vCells;       /** < number of cells in V direction. */

  }; // class B2GeomSVDSensorActive

  class B2GeomSVDSensorSilicon : public B2GeomVXDVolume {
  public:
    /** Constructor for geometry object representing the silicon of the SVD sensor. */
    B2GeomSVDSensorSilicon();
    /** Destructor for geometry object representing the silicon of the SVD sensor. */
    virtual ~B2GeomSVDSensorSilicon() {}
    /** Initialize Parameters from GearBox
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    Bool_t init(GearDir& content);
    /** Build TGeoVolume according to the parameters. */
    Bool_t make();
  private:
    B2GeomSVDSensorActive* volActive; /** < Object representing the geometry of the active sensor of SVD. */

  }; // class B2GeomSVDSensorSilicon

  class B2GeomSVDSensor : public B2GeomVXDStructVolume<B2GeomSVDSensor> {
  public:
    /** Constructor for geometry object representing the SVD active sensor. */
    B2GeomSVDSensor();
    /** Destructor for geometry object representing the SVD active sensor. */
    virtual ~B2GeomSVDSensor() {}
    /** Initialize Parameters from GearBox
     * @param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
     */
    Bool_t init(GearDir& content);
    /** Build TGeoVolume according to the parameters. */
    Bool_t make();

  private:
    B2GeomSVDSensorSilicon* volSilicon; /** < Object representing the geometry of the silicon of the SVD sensor. */
    B2GeomVXDVolume* volFoam; /** < Object representing the geometry of the insulating Rohacell Foam pad of the SVD sensor. */
    B2GeomVXDVolume* volSMDs; /** < Object representing the geometry of the chips on the SVD sensor */
    B2GeomVXDVolume* volKapton; /** < Object to represent the geometry of one type of kapton cable. */
    B2GeomVXDVolume* volKaptonFlex; /** < Object to represent the geometry of one type of kapton cable. */
    B2GeomVXDVolume* volKaptonFront; /** < Object to represent the geometry of one type of kapton cable. */

  }; // class B2GeomSVDSensor

} // namespace Belle2

#endif /* B2GEOMSVDSENSOR_H */
