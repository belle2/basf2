/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2GEOMPXDLAYER_H_
#define B2GEOMPXDLAYER_H_

#include <pxd/geopxd/B2GeomVolume.h>
#include <pxd/geopxd/B2GeomPXDLadder.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <boost/format.hpp>

#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TVector3.h"

#include <vector>
#include <string>

using namespace std;

namespace Belle2 {

  class GearDir;

  class B2GeomPXDLayer : public B2GeomVXDStructVolume<B2GeomPXDLadder> {
  private:
    Double_t fPhi0;     /** < Rotation of whole PXD about z axis */
    vector<Double_t> fPhi; /** < Rotation about z axis after moving to fRadius for each individual ladder */
  public:
    /** Constructor for a PXD layer */
    B2GeomPXDLayer() {}
    /** Destructor for a PXD layer */
    virtual ~B2GeomPXDLayer() {}
    /** initialize this ladder with parameters from GearDir. */
    Bool_t init(GearDir& content);
    /** Build a PXD ladder after initializing parameters */
    Bool_t make();
  };
}
#endif
