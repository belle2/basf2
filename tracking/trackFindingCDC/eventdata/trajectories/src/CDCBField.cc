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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;



const FloatType& TrackFindingCDC::getBFieldZMagnitude(const Vector2D&)
{
  return c_bFieldZMagnitude;
}



const SignType& TrackFindingCDC::getBFieldZSign()
{
  return c_bFieldZSign;
}



const FloatType& TrackFindingCDC::getBFieldZ(const Vector2D&)
{
  return c_bFieldZ;
}



const FloatType& TrackFindingCDC::getBFieldZ(const Vector3D&)
{
  return c_bFieldZ;
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



FloatType TrackFindingCDC::absMom2DToRadius(const FloatType& absMom2D,
                                            const FloatType& charge,
                                            const Vector2D& pos2D)
{
  return - absMom2D / (charge * getBFieldZ(pos2D) * 0.00299792458);
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
                                               const FloatType& charge,
                                               const Vector2D& pos2D)
{
  return  - charge *  getBFieldZ(pos2D) * 0.00299792458 / curvature ;
}

