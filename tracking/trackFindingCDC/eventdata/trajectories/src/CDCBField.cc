/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/logging/Logger.h>

#include <TMath.h>
#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

double TrackFindingCDC::getBFieldZMagnitude(const Vector2D& pos2D)
{
  return std::fabs(TrackFindingCDC::getBFieldZ(pos2D));
}

ESign TrackFindingCDC::getBFieldZSign()
{
  return sign(TrackFindingCDC::getBFieldZ());
}

double TrackFindingCDC::getBFieldZ(const Vector2D& pos2D)
{
  return getBFieldZ(Vector3D(pos2D, 0));
}

double TrackFindingCDC::getBFieldZ(const Vector3D& pos3D)
{
  // The BFieldMap can not handle positions with not a number coordinates
  // which can occure if fits fail.
  // Return NAN to the caller and let him decide what to do next.
  if (pos3D.hasNAN()) return NAN;
  TVector3 mag3D = BFieldMap::Instance().getBField(pos3D);
  return mag3D.Z();
}

double TrackFindingCDC::getAlphaFromBField(const double bField)
{
  return 1.0 / (bField * TMath::C()) * 1E11;
}

double TrackFindingCDC::getAlphaZ(const Vector2D& pos2D)
{
  return getAlphaFromBField(getBFieldZ(pos2D));
}

double TrackFindingCDC::getAlphaZ(const Vector3D& pos3D)
{
  return getAlphaFromBField(getBFieldZ(pos3D));
}

ESign TrackFindingCDC::ccwInfoToChargeSign(const ERotation ccwInfo)
{
  return static_cast<ESign>(- ccwInfo * getBFieldZSign());
}

ERotation TrackFindingCDC::chargeSignToERotation(const ESign& chargeSign)
{
  return static_cast<ERotation>(- chargeSign * getBFieldZSign());
}

ERotation TrackFindingCDC::chargeToERotation(const double charge)
{
  return chargeSignToERotation(sign(charge));
}

double TrackFindingCDC::absMom2DToBendRadius(const double absMom2D,
                                             const double bZ)
{
  return absMom2D / (bZ * 0.00299792458);
}

double TrackFindingCDC::absMom2DToBendRadius(const double absMom2D,
                                             const Vector2D& pos2D)
{
  return absMom2DToBendRadius(absMom2D, getBFieldZ(pos2D));
}

double TrackFindingCDC::absMom2DToBendRadius(const double absMom2D,
                                             const Vector3D& pos3D)
{
  return absMom2DToBendRadius(absMom2D, getBFieldZ(pos3D));
}

double TrackFindingCDC::absMom2DToCurvature(const double absMom2D,
                                            const double charge,
                                            const double bZ)
{
  return - charge * bZ * 0.00299792458 / absMom2D;
}

double TrackFindingCDC::absMom2DToCurvature(const double absMom2D,
                                            const double charge,
                                            const Vector2D& pos2D)
{
  return absMom2DToCurvature(absMom2D, charge, getBFieldZ(pos2D));
}

double TrackFindingCDC::absMom2DToCurvature(const double absMom2D,
                                            const double charge,
                                            const Vector3D& pos3D)
{
  return absMom2DToCurvature(absMom2D, charge, getBFieldZ(pos3D));
}

double TrackFindingCDC::curvatureToAbsMom2D(const double curvature,
                                            const double bZ)
{
  return std::fabs(bZ * 0.00299792458 / curvature);
}

double TrackFindingCDC::curvatureToAbsMom2D(const double curvature,
                                            const Vector2D& pos2D)
{
  return curvatureToAbsMom2D(curvature, getBFieldZ(pos2D));
}

double TrackFindingCDC::curvatureToAbsMom2D(const double curvature,
                                            const Vector3D& pos3D)
{
  return curvatureToAbsMom2D(curvature, getBFieldZ(pos3D));
}
