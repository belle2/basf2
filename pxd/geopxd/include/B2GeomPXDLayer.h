/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef B2GEOM_BASF2
#include <pxd/geopxd/B2GeomPXDLadder.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#else
#include "B2GeomPXDLadder.h"
#endif

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

#ifdef B2GEOM_BASF2
namespace Belle2 {

  class GearDir;
#endif
  class B2GeomPXDLayer {

  private:

#ifdef B2GEOM_BASF2
    GearDir layerContent;
#endif
    //! path of this Layer
    string path;

    //! number of the Layer - 1
    Int_t iLayer;
    //! number of ladders per layer
    Int_t nLadders;
    //! Rotation of whole PXD about z axis
    Double_t fPhi0;
    //! Rotation about z axis before moving to fRadius
    vector<Double_t> fTheta;
    //! Distance from origin
    Double_t fRadius;
    //! Rotation about z axis after moving to fRadius
    vector<Double_t> fPhi;
    //! Offset in z direction
    Double_t fLadderOffsetZ;
    //! Offset in y direction (wind mill structure)
    Double_t fLadderOffsetY;
    //! which layer is in which half shell?
    vector<Int_t> iHalfShell;

    vector<B2GeomPXDLadder*> b2gPXDLadders;
    TGeoVolumeAssembly* volPXDLayer;

    void putLadder(Int_t iLadder);


  public:
    B2GeomPXDLayer();
    B2GeomPXDLayer(Int_t iLay);
    ~B2GeomPXDLayer();
#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
#else
    Bool_t init();
#endif
    Bool_t make();
    TGeoVolume* getVol() {
      return volPXDLayer;

    }
  };
#ifdef B2GEOM_BASF2
}
#endif
#endif
