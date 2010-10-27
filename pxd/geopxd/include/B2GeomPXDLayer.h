/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/geopxd/B2GeomPXDLadder.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <pxd/geopxd/B2GeomVolume.h>
#include <boost/format.hpp>


#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TVector3.h"

#include <vector>
#include <string>


#ifndef B2GEOMPXDLAYER_H_
#define B2GEOMPXDLAYER_H_

using namespace std;

namespace Belle2 {

  class GearDir;

  class B2GeomPXDLayer : public B2GeomVolume {

  private:
    //! path of this Layer
    string path;

    //! number of the Layer - 1
    Int_t iLayer;
    //! number of ladders per layer
    Int_t nLadders;
    //! Rotation of whole PXD about z axis
    Double_t fPhi0;
    //! Rotation about z axis after moving to fRadius
    vector<Double_t> fPhi;
    B2GeomPXDLadder** b2gPXDLadders;

  public:
    B2GeomPXDLayer();
    B2GeomPXDLayer(Int_t iLay);
    ~B2GeomPXDLayer();
    Bool_t init(GearDir& content);
    Bool_t make();

  };
}
#endif
