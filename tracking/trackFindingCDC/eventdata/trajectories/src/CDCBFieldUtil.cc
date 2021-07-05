/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBFieldUtil.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>

#include <tracking/trackFindingCDC/numerics/ERotation.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>

#include <framework/geometry/BFieldManager.h>

#include <framework/geometry/B2Vector3.h>

#include <TMath.h>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

bool CDCBFieldUtil::isOff()
{
  B2Vector3D origin(0, 0, 0);
  double b = BFieldManager::getField(origin).Mag();
  double c_EarthMagneticField = 3.2e-5 * Unit::T;
  return not(b > 5 * c_EarthMagneticField);
}

ESign CDCBFieldUtil::getBFieldZSign()
{
  return sign(CDCBFieldUtil::getBFieldZ());
}

double CDCBFieldUtil::getBFieldZ()
{
  return getBFieldZ(Vector3D(0, 0, 0));
}

double CDCBFieldUtil::getBFieldZ(const Vector2D& pos2D)
{
  return getBFieldZ(Vector3D(pos2D, 0));
}

double CDCBFieldUtil::getBFieldZ(const Vector3D& pos3D)
{
  // The BFieldMap can not handle positions with not a number coordinates
  // which can occure if fits fail.
  // Return NAN to the caller and let him decide what to do next.
  if (pos3D.hasNAN()) return NAN;
  auto mag3D = BFieldManager::getField(pos3D.x(), pos3D.y(), pos3D.z());
  return mag3D.Z() / Unit::T;
}



double CDCBFieldUtil::getAlphaFromBField(double bField)
{
  return 1.0 / (bField * TMath::C()) * 1E11;
}

double CDCBFieldUtil::getAlphaZ(const Vector2D& pos2D)
{
  return getAlphaFromBField(getBFieldZ(pos2D));
}

double CDCBFieldUtil::getAlphaZ(const Vector3D& pos3D)
{
  return getAlphaFromBField(getBFieldZ(pos3D));
}

ESign CDCBFieldUtil::ccwInfoToChargeSign(ERotation ccwInfo)
{
  return static_cast<ESign>(- ccwInfo * getBFieldZSign());
}

ERotation CDCBFieldUtil::chargeSignToERotation(ESign chargeSign)
{
  return static_cast<ERotation>(- chargeSign * getBFieldZSign());
}

ERotation CDCBFieldUtil::chargeToERotation(double charge)
{
  return chargeSignToERotation(sign(charge));
}

double CDCBFieldUtil::absMom2DToBendRadius(double absMom2D,
                                           double bZ)
{
  // In case of zero magnetic field return something large
  return std::fmin(4440, absMom2D / (bZ * 0.00299792458));
}

double CDCBFieldUtil::absMom2DToBendRadius(double absMom2D,
                                           const Vector2D& pos2D)
{
  return absMom2DToBendRadius(absMom2D, getBFieldZ(pos2D));
}

double CDCBFieldUtil::absMom2DToBendRadius(double absMom2D,
                                           const Vector3D& pos3D)
{
  return absMom2DToBendRadius(absMom2D, getBFieldZ(pos3D));
}

double CDCBFieldUtil::absMom2DToCurvature(double absMom2D,
                                          double charge,
                                          double bZ)
{
  return - charge * bZ * 0.00299792458 * std::fmax(0, 1 / absMom2D);
}

double CDCBFieldUtil::absMom2DToCurvature(double absMom2D,
                                          double charge,
                                          const Vector2D& pos2D)
{
  return absMom2DToCurvature(absMom2D, charge, getBFieldZ(pos2D));
}

double CDCBFieldUtil::absMom2DToCurvature(double absMom2D,
                                          double charge,
                                          const Vector3D& pos3D)
{
  return absMom2DToCurvature(absMom2D, charge, getBFieldZ(pos3D));
}

double CDCBFieldUtil::curvatureToAbsMom2D(double curvature,
                                          double bZ)
{
  return std::fmin(20, std::fabs(bZ * 0.00299792458 / curvature));
}

double CDCBFieldUtil::curvatureToAbsMom2D(double curvature,
                                          const Vector2D& pos2D)
{
  return curvatureToAbsMom2D(curvature, getBFieldZ(pos2D));
}

double CDCBFieldUtil::curvatureToAbsMom2D(double curvature,
                                          const Vector3D& pos3D)
{
  return curvatureToAbsMom2D(curvature, getBFieldZ(pos3D));
}
