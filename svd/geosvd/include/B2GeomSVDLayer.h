/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDLadder.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
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
  class B2GeomSVDLayer : public B2GeomVolume {

  private:
    //! path of this Layer
    string path;

    //! number of the Layer
    Int_t iLayer;
    //! number of ladders per layer
    Int_t nLadders;
    //! Rotation of whole SVD about z axis
    Double_t fPhi0;
    //! Rotation about z axis before moving to fRadius
    vector<Double_t> fTheta;
    //! Rotation about z axis after moving to fRadius
    vector<Double_t> fPhi;

    //! rotation of the ladder about its local v axis (parallel to global z axis)
    Double_t fPhiLadder;

    //! Objects representing the ladders
    B2GeomSVDLadder** b2gSVDLadders;

  public:
    B2GeomSVDLayer();
    B2GeomSVDLayer(Int_t iLay);
    ~B2GeomSVDLayer();

    Bool_t init(GearDir& content);
    Bool_t make();

  };
}
#endif
