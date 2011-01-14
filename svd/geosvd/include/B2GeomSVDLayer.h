/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDLadder.h>
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
#include <string>
#include <vector>


#ifndef B2GEOMSVDLAYER_H_
#define B2GEOMSVDLAYER_H_

using namespace std;

namespace Belle2 {

  class GearDir;
  class B2GeomSVDLayer : public B2GeomVXDStructVolume<B2GeomSVDLadder> {

  private:
    Double_t fPhi0; /** < starting angle for placing the ladders. */
  public:
    /** Constructor. */
    B2GeomSVDLayer();
    /** Initialize parameters from GearBox. */
    Bool_t init(GearDir& content);
    /** Build the geometry. */
    Bool_t make();
  };
}
#endif
