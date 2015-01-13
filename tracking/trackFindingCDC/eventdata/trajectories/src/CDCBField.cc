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

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;



const FloatType& CDCLocalTracking::getBFieldZMagnitude(const Vector2D&)
{
  return c_bFieldZMagnitude;
}



const SignType& CDCLocalTracking::getBFieldZSign()
{
  return c_bFieldZSign;
}



const FloatType& CDCLocalTracking::getBFieldZ(const Vector2D&)
{
  return c_bFieldZ;
}



const FloatType& CDCLocalTracking::getBFieldZ(const Vector3D&)
{
  return c_bFieldZ;
}



SignType CDCLocalTracking::ccwInfoToChargeSign(const CCWInfo& ccwInfo)
{
  return - ccwInfo * getBFieldZSign();
}



CCWInfo CDCLocalTracking::chargeSignToCCWInfo(const SignType& chargeSign)
{
  return - chargeSign * getBFieldZSign();
}



CCWInfo CDCLocalTracking::chargeToCCWInfo(const FloatType& charge)
{
  return chargeSignToCCWInfo(sign(charge));
}



FloatType CDCLocalTracking::absMom2DToRadius(const FloatType& absMom2D,
                                             const FloatType& charge,
                                             const Vector2D& pos2D)
{
  return - absMom2D / (charge * getBFieldZ(pos2D) * 0.00299792458);
}



FloatType CDCLocalTracking::absMom2DToCurvature(const FloatType& absMom2D,
                                                const FloatType& charge,
                                                const Vector2D& pos2D)
{
  return - charge * getBFieldZ(pos2D) * 0.00299792458 / absMom2D;
}



FloatType CDCLocalTracking::absMom2DToCurvature(const FloatType& absMom2D,
                                                const FloatType& charge,
                                                const Vector3D& pos3D)
{
  return - charge * getBFieldZ(pos3D) * 0.00299792458 / absMom2D;
}



FloatType CDCLocalTracking::curvatureToAbsMom2D(const FloatType& curvature,
                                                const Vector2D& pos2D)
{
  return std::fabs(getBFieldZ(pos2D) * 0.00299792458 / curvature);
}



FloatType CDCLocalTracking::curvatureToAbsMom2D(const FloatType& curvature,
                                                const FloatType& charge,
                                                const Vector2D& pos2D)
{
  return  - charge *  getBFieldZ(pos2D) * 0.00299792458 / curvature ;
}

