/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dbobjects/PXDSensorInfoPar.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/gearbox/Const.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace Belle2;
using namespace std;



// Get VXD geometry parameters from Gearbox (no calculations here)
// *** This is a DIVOT ***
/*
void PXDSensorInfoPar::read(const GearDir& sensor)
{
}
*/


double PXDSensorInfoPar::getElectronMobility(double E) const
{
  // Electron parameters - maximum velocity, critical intensity, beta factor
  static double vmElec = 1.53 * pow(m_temperature, -0.87) * 1.E9 * Unit::cm
                         / Unit::s;
  static double EcElec = 1.01 * pow(m_temperature, +1.55) * Unit::V
                         / Unit::cm;
  static double betaElec = 2.57 * pow(m_temperature, +0.66) * 1.E-2;

  return (vmElec / EcElec * 1.
          / (pow(1. + pow((fabs(E) / EcElec), betaElec), (1. / betaElec))));
}

const TVector3 PXDSensorInfoPar::getEField(const TVector3& point) const
{
  // FIXME: Get rid of the gateDepth
  double depletionVoltage = 0.5 * Unit::e * m_bulkDoping
                            / Const::permSi * m_thickness * m_thickness;
  double gateZ = 0.5 * m_thickness - m_gateDepth;
  double Ez = 2 * depletionVoltage * (point.Z() - gateZ) / m_thickness
              / m_thickness;
  TVector3 E(0, 0, Ez);
  return E;
}


/*
const TVector3 PXDSensorInfoPar::getBField(const TVector3& point) const
{
  TVector3 pointGlobal = pointToGlobal(point);
  TVector3 bGlobal = BFieldMap::Instance().getBField(pointGlobal);
  TVector3 bLocal = vectorToLocal(bGlobal);
  return Unit::T * bLocal;
}
*/

const TVector3 PXDSensorInfoPar::getDriftVelocity(const TVector3& E,
                                                  const TVector3& B) const
{
  // Set mobility parameters
  double mobility = -getElectronMobility(E.Mag());
  double mobilityH = m_hallFactor * mobility;
  // Calculate products
  TVector3 EcrossB = E.Cross(B);
  TVector3 BEdotB = E.Dot(B) * B;
  TVector3 v = mobility * E + mobility * mobilityH * EcrossB
               + mobility * mobilityH * mobilityH * BEdotB;
  v *= 1.0 / (1.0 + mobilityH * mobilityH * B.Mag2());
  return v;
}

/*
const TVector3 PXDSensorInfoPar::getLorentzShift(double u, double v) const
{
  // Constants for the 5-point Gauss quadrature formula
  const double alphaGL = 1.0 / 3.0 * sqrt(5.0 + 2.0 * sqrt(10.0 / 7.0));
  const double betaGL = 1.0 / 3.0 * sqrt(5.0 - 2.0 * sqrt(10.0 / 7.0));
  const double walpha = (322 - 13.0 * sqrt(70)) / 900;
  const double wbeta = (322 + 13.0 * sqrt(70)) / 900;
  const double distanceToPlane = 0.5 * m_thickness - m_gateDepth;
  const double midpoint = 0.5 * distanceToPlane;
  const double h = 0.5 * distanceToPlane;
  const double weightGL[5] = {
    h * walpha, h * wbeta, h * 128.0 / 225.0, h * wbeta, h* walpha
  };
  const double zKnots[5] = {
    midpoint - alphaGL * h, midpoint - betaGL * h, midpoint, midpoint + betaGL * h, midpoint + alphaGL* h
  };
  // Integrate v/v_z from z = 0 to z = distanceToPlane
  TVector3 position(u, v, 0);
  TVector3 currentBField = getBField(position);
  for (int iz = 0; iz < 5; ++iz) {
    // This is OK as long as E only depends on z
    TVector3 currentEField = getEField(TVector3(0, 0, zKnots[iz]));
    TVector3 v = getDriftVelocity(currentEField, currentBField);
    position += weightGL[iz] / v.Z() * v;
  } // for knots
  position.SetZ(0);
  position.SetX(position.X() - u);
  position.SetY(position.Y() - v);
  return position;
}
*/

