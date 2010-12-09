/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDSensor.h>
#include <pxd/geopxd/B2GeomVolume.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>
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
namespace Belle2 {
  class GearDir;


  /** This class contains the builder of the geometry of the carbon of a SVD ladder rib.
   *  The rib consists of a foam piece placed inside a carbon piece which is equivalent to
   *  1 carbon rib + 1 foam piece + 1 carbon rib
   *  CCCCCCCCCCCC + FFFFFFFFF = CC|FFFFFFFF|CC
   */
  class B2GeomSVDLadderRib : public B2GeomVXDVolume {
  protected:
    B2GeomVXDVolume* volFoam; /** < The Rohacell foam piece */
  public:
    /** Constructor */
    B2GeomSVDLadderRib();
    /** Initializes the parameters from GearBox */
    Bool_t init(GearDir& content);
    /** builds the geometry of the rib */
    Bool_t make();
  };

  /** This class contains the builder of the geometry of the set of two SVD ladder ribs. */
  class B2GeomSVDLadderRibs : public B2GeomVXDVolume {
  protected:
    B2GeomSVDLadderRib* volRib; /** < Builds the geometry of the two equivalent ribs */
    Float_t fRib1UPosition; /** < Position of the first rib inside the box containing both ribs */
    Float_t fRib2UPosition; /** < Position of the second rib inside the box containing both ribs */
  public:
    /** Creator */
    B2GeomSVDLadderRibs();
    /** Initializes the parameters from GearBox */
    Bool_t init(GearDir& content);
    /** builds the geometry of the ribs */
    Bool_t make();
  };


  /** Class describing the cooling pipe of SVD (metal + included liquid) */
  class B2GeomSVDLadderCoolingpipe : public B2GeomVXDVolume {
  protected:
    B2GeomVXDVolume* volLiquid; /** < Object building the geometry of the cooling liquid*/
  public:
    /** Constructor. */
    B2GeomSVDLadderCoolingpipe();
    /** Initializes the parameters from GearBox */
    Bool_t init(GearDir& content);
    /** Builds the geometry of the cooling pipe. */
    Bool_t make();
  };

  /** Classes describing the SVD ladder.
   *  The SVD ladder consists of two Ribs in barrel and slanted region, the cooling pipe and the sensors.
   */
  class B2GeomSVDLadder : public B2GeomVXDStructVolume<B2GeomSVDSensor> {
  private:
    B2GeomSVDLadderRibs* volBarrelRibs; /** < The ribs in the barrel region */
    B2GeomSVDLadderRibs* volSlantedRibs; /** < The ribs in the slanted region */
    B2GeomSVDLadderCoolingpipe* volCoolingpipe; /** < The cooling pipe (which contains the cooling liquid). */
  public:
    /** Constructor. */
    B2GeomSVDLadder();
    /** Destructor. */
    ~B2GeomSVDLadder() { }
    /** Initializes the parameters from GearBox. */
    Bool_t init(GearDir& content);
    /** Builds the geometry of the SVD ladder. */
    Bool_t make();
  };
}
#endif

