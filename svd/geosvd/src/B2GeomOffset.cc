/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#define B2GEOM_BASF2

#ifdef B2GEOM_BASF2
#include <svd/geosvd/B2GeomOffset.h>
using namespace boost;
using namespace Belle2;
#else
#include "B2GeomOffset.h"
#endif

B2GeomOffset::B2GeomOffset()
{
  fOffsetW = 1e25;
  fOffsetU = 1e25;
  fOffsetV = 1e25;
  fOffsetPhi = 0;
  fOffsetTheta = 0;
  fOffsetPsi = 0;
  hmaOffset = NULL;
}

B2GeomOffset::~B2GeomOffset()
{

}

#ifdef B2GEOM_BASF2
Bool_t B2GeomOffset::init(GearDir offsetDir)
{
  fOffsetPhi = offsetDir.getParamAngle("OffsetPhi");
  fOffsetTheta = offsetDir.getParamAngle("OffsetTheta");
  fOffsetPsi = offsetDir.getParamAngle("OffsetPsi");
  fOffsetW = offsetDir.getParamLength("OffsetW");
  fOffsetU = offsetDir.getParamLength("OffsetU");
  fOffsetV = offsetDir.getParamLength("OffsetV");
}
#else
Boolt_t B2GeomOffset::init()
{
  fOffsetW = 0.0;
  fOffsetU = 0.0;
  fOffsetV = 0.0;
  fOffsetPhi = 0.;
  fOffsetTheta = 0.0;
  fOffsetPsi = 0.0;
}
#endif

TGeoHMatrix* B2GeomOffset::getHMatrix()
{
  // check if offset matrix has already been created
  if (hmaOffset != NULL) {
    return hmaOffset;
  }

  // first do the rotation, then the translation
  TGeoRotation rot("B2GeomOffset Rotation", fOffsetPhi, fOffsetTheta, fOffsetPsi);
  TGeoTranslation tra(fOffsetW, fOffsetU, fOffsetV);
  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  hmaHelp = rot * hmaHelp;
  hmaHelp = tra * hmaHelp;

  TGeoHMatrix* hmaOffset = new TGeoHMatrix(hmaHelp);

  return hmaOffset;
}
