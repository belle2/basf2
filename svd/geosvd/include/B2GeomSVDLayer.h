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
#include <svd/geosvd/B2GeomSVDLadder.h>
#include <svd/geosvd/B2GeomOffset.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#else
#include "B2GeomSVDLadder.h"
#include "B2GeomOffset.h"
#endif

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

#ifdef B2GEOM_BASF2
namespace Belle2 {

  class GearDir;
#endif
  class B2GeomSVDLayer {

  private:
#ifdef B2GEOM_BASF2
    GearDir layerContent;
#endif
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
    //! Distance from origin
    Double_t fRadius;
    //! Rotation about z axis after moving to fRadius
    vector<Double_t> fPhi;
    //! Offset in y direction (wind mill structure)
    Double_t fLadderOffsetY;

    //! rotation of the ladder about its local v axis (parallel to global z axis)
    Double_t fPhiLadder;

    //! Objects representing the ladders
    vector<B2GeomSVDLadder*> b2gSVDLadders;
    //! Object representing the offsets of the ladders
    vector<B2GeomOffset*> b2gLadderOffsets;

    TGeoVolumeAssembly* volSVDLayer;

    void putLadder(Int_t iLadder);


  public:
    B2GeomSVDLayer();
    B2GeomSVDLayer(Int_t iLay);
    ~B2GeomSVDLayer();
#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
    Bool_t initOffsets();
#else
    Bool_t init();
#endif
    Bool_t make();
    TGeoVolume* getVol() {
      return volSVDLayer;

    }
  };
#ifdef B2GEOM_BASF2
}
#endif
#endif
