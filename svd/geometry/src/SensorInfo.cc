/*
 * SensorInfo.cc
 *
 *  Created on: Dec 21, 2013
 *      Author: kvasnicka
 */

#include <svd/geometry/SensorInfo.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/BFieldManager.h>

#include <TVector3.h>
#include <math.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;


double SensorInfo::getElectronMobility(double E) const
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

double SensorInfo::getHoleMobility(double E) const
{
  // Hole parameters - maximum velocity, critical intensity, beta factor
  static double vmHole = 1.62 * pow(m_temperature, -0.52) * 1.E8 * Unit::cm
                         / Unit::s;
  static double EcHole = 1.24 * pow(m_temperature, +1.68) * Unit::V
                         / Unit::cm;
  static double betaHole = 0.46 * pow(m_temperature, +0.17);

  return (vmHole / EcHole * 1.
          / (pow(1. + pow((fabs(E) / EcHole), betaHole), (1. / betaHole))));
}

const TVector3 SensorInfo::getEField(const TVector3& point) const
{

  TVector3 E(0, 0,
             - 2.0 * m_depletionVoltage / m_thickness
             * ((point.Z() + 0.5 * m_thickness) / m_thickness)
             - (m_biasVoltage - m_depletionVoltage) / m_thickness);

  return E;
}

const TVector3& SensorInfo::getBField(const TVector3& point) const
{
  static TVector3 oldPoint(0, 0, 1000 * Unit::cm);
  static TVector3 oldField(0, 0, 0);
  static double bRadius = 0.5 * Unit::cm;
  if (TVector3(point - oldPoint).Mag() > bRadius) { // renew if far point
    TVector3 pointGlobal = pointToGlobal(point);
    TVector3 bGlobal = BFieldManager::getField(pointGlobal);
    TVector3 bLocal = vectorToLocal(bGlobal);
    oldPoint = point;
    oldField = bLocal;
  }
  return oldField;
}

const TVector3 SensorInfo::getVelocity(CarrierType carrier,
                                       const TVector3& point) const
{
  TVector3 E = getEField(point);
  // This is what makes the digitizer slow.
  TVector3 B = getBField(point);
  // Set mobility parameters
  double mobility = 0;
  double hallFactor = getHallFactor(carrier);
  if (carrier == electron) {
    mobility = -getElectronMobility(E.Mag());
  } else {
    mobility = getHoleMobility(E.Mag());
  }
  double mobilityH = hallFactor * mobility;
  // Calculate products
  TVector3 EcrossB = E.Cross(B);
  TVector3 BEdotB = E.Dot(B) * B;
  TVector3 velocity = mobility * E + mobility * mobilityH * EcrossB
                      + +mobility * mobilityH * mobilityH * BEdotB;
  velocity *= 1.0 / (1.0 + mobilityH * mobilityH * B.Mag2());
  return velocity;
}

const TVector3& SensorInfo::getLorentzShift(double uCoord, double vCoord) const
{
  static TVector3 result;
  double distanceToFrontPlane = 0.5 * m_thickness;
  double distanceToBackPlane = 0.5 * m_thickness;

  // Approximation: calculate drift velocity at the point halfway towards
  // the respective sensor surface.
  TVector3 v_e = getVelocity(electron, TVector3(uCoord, vCoord, +0.5 * distanceToFrontPlane));
  TVector3 v_h = getVelocity(hole, TVector3(uCoord, vCoord, -0.5 * distanceToBackPlane));

  // Calculate drift directions
  TVector3 center_e = fabs(distanceToFrontPlane / v_e.Z()) * v_e;
  TVector3 center_h = fabs(distanceToBackPlane / v_h.Z()) * v_h;
  result.SetXYZ(center_h.X(), center_e.Y(), 0.0);
  return result;
}

double SensorInfo::getLorentzShift(bool isUCoordinate, double position) const
{
  if (isUCoordinate)
    return getLorentzShift(position, 0.0).X();
  else
    return getLorentzShift(0.0, position).Y();
}

