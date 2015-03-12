/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCBField.h"

#include "TMath.h"
#include <cmath>
#include <cassert>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FloatType TrackFindingCDC::getBFieldZMagnitude(const Vector2D& pos2D)
{
  return std::fabs(TrackFindingCDC::getBFieldZ(pos2D));
}



SignType TrackFindingCDC::getBFieldZSign()
{
  return sign(TrackFindingCDC::getBFieldZ());
}



FloatType TrackFindingCDC::getBFieldZ(const Vector2D& pos2D)
{
  return getBFieldZ(Vector3D(pos2D, 0));
}



FloatType TrackFindingCDC::getBFieldZ(const Vector3D& pos3D)
{
  TVector3 mag3D = BFieldMap::Instance().getBField(pos3D);
  return mag3D.Z();
}


FloatType TrackFindingCDC::getAlphaFromBField(const double& bField)
{
  return 1.0 / (bField * TMath::C()) * 1E11;
}

FloatType TrackFindingCDC::getAlphaZ(const Vector2D& pos2D)
{
  return getAlphaFromBField(getBFieldZ(pos2D));
}

FloatType TrackFindingCDC::getAlphaZ(const Vector3D& pos3D)
{
  return getAlphaFromBField(getBFieldZ(pos3D));
}

SignType TrackFindingCDC::ccwInfoToChargeSign(const CCWInfo& ccwInfo)
{
  return - ccwInfo * getBFieldZSign();
}



CCWInfo TrackFindingCDC::chargeSignToCCWInfo(const SignType& chargeSign)
{
  return - chargeSign * getBFieldZSign();
}



CCWInfo TrackFindingCDC::chargeToCCWInfo(const FloatType& charge)
{
  return chargeSignToCCWInfo(sign(charge));
}


FloatType TrackFindingCDC::absMom2DToCurvature(const FloatType& absMom2D,
                                               const FloatType& charge,
                                               const Vector2D& pos2D)
{
  return - charge * getBFieldZ(pos2D) * 0.00299792458 / absMom2D;
}

FloatType TrackFindingCDC::absMom2DToCurvature(const FloatType& absMom2D,
                                               const FloatType& charge,
                                               const Vector3D& pos3D)
{
  return - charge * getBFieldZ(pos3D) * 0.00299792458 / absMom2D;
}

FloatType TrackFindingCDC::curvatureToAbsMom2D(const FloatType& curvature,
                                               const Vector2D& pos2D)
{
  return std::fabs(getBFieldZ(pos2D) * 0.00299792458 / curvature);
}


FloatType TrackFindingCDC::curvatureToAbsMom2D(const FloatType& curvature,
                                               const Vector3D& pos3D)
{
  return std::fabs(getBFieldZ(pos3D) * 0.00299792458 / curvature);
}



FloatType TrackFindingCDC::curvatureToAbsMom2D(const FloatType& curvature,
                                               const FloatType& charge,
                                               const Vector2D& pos2D)
{
  return  - charge *  getBFieldZ(pos2D) * 0.00299792458 / curvature ;
}

