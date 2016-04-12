/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012, 2015 - Belle II Collaboration                       *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck, Tobias Schlüter                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/reconstruction/AlignableSVDRecoHit.h>

#include <alignment/GlobalLabel.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/database/DBObjPtr.h>
#include <alignment/dbobjects/VXDAlignment.h>

#include <vxd/geometry/GeoCache.h>

#include <vxd/geometry/SensorPlane.h>
#include <TGeoMatrix.h>

using namespace std;
using namespace Belle2;

TMatrixD AlignableSVDRecoHit::derivatives(const genfit::StateOnPlane* sop)
{

  // values for global derivatives
  //TMatrixD derGlobal(2, 6);
  TMatrixD derGlobal(2, 6);
  derGlobal.Zero();

  // track u-slope in local sensor system
  double uSlope = sop->getState()[1];
  // track v-slope in local sensor system
  double vSlope = sop->getState()[2];
  // Predicted track u-position in local sensor system
  double uPos = sop->getState()[3];
  // Predicted track v-position in local sensor system
  double vPos = sop->getState()[4];

  //Global derivatives for alignment in sensor local coordinates

  derGlobal(0, 0) = 1.0;
  derGlobal(0, 1) = 0.0;
  derGlobal(0, 2) = - uSlope;
  derGlobal(0, 3) = vPos * uSlope;
  derGlobal(0, 4) = -uPos * uSlope;
  derGlobal(0, 5) = vPos;

  derGlobal(1, 0) = 0.0;
  derGlobal(1, 1) = 1.0;
  derGlobal(1, 2) = - vSlope;
  derGlobal(1, 3) = vPos * vSlope;
  derGlobal(1, 4) = -uPos * vSlope;
  derGlobal(1, 5) = -uPos;

  return derGlobal;

}

vector< int > AlignableSVDRecoHit::labels()
{
  VxdID vxdid(getPlaneId());

  std::vector<int> labGlobal;

  labGlobal.push_back(GlobalLabel(vxdid, VXDAlignment::dU)); // du
  labGlobal.push_back(GlobalLabel(vxdid, VXDAlignment::dV)); // dv
  labGlobal.push_back(GlobalLabel(vxdid, VXDAlignment::dW)); // dw
  labGlobal.push_back(GlobalLabel(vxdid, VXDAlignment::dAlpha)); // dalpha
  labGlobal.push_back(GlobalLabel(vxdid, VXDAlignment::dBeta)); // dbeta
  labGlobal.push_back(GlobalLabel(vxdid, VXDAlignment::dGamma)); // dgamma

  return labGlobal;
}
