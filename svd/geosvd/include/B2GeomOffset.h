/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef B2GEOM_BASF2
#include <geometry/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#endif

#include "TGeoMatrix.h"

#ifndef B2GEOMOFFSET_H
#define B2GEOMOFFSET_H

using namespace std;

#ifdef B2GEOM_BASF2
namespace Belle2 {

  class GearDir;
#endif

  class B2GeomOffset {
  private:
    // offsets according to local coordinates of component (in cm)
    Double_t fOffsetW;
    Double_t fOffsetU;
    Double_t fOffsetV;
    Double_t fOffsetPhi;
    Double_t fOffsetTheta;
    Double_t fOffsetPsi;

    //! The matrix which corresponds to the offset
    TGeoHMatrix* hmaOffset;

  public:
    B2GeomOffset();
    ~B2GeomOffset();

#ifdef B2GEOM_BASF2
    //! initialises the parameters from GearDir
    Bool_t init(GearDir offSetDir);
#else
    Bool_t init();
#endif

    //! returns the TGeoHMatrix corresponding to the offset of the component
    TGeoHMatrix* getHMatrix();
  };

#ifdef B2GEOM_BASF2
}
#endif

#endif
