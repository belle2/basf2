/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/geometry/SensorInfo.h>
#include <framework/gearbox/Unit.h>
#include <framework/geometry/BFieldManager.h>

#include <Math/Vector3D.h>
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

const ROOT::Math::XYZVector SensorInfo::getEField(const ROOT::Math::XYZVector& point) const
{

  ROOT::Math::XYZVector E(0, 0,
                          + 2.0 * m_depletionVoltage / m_thickness
                          * ((+point.Z() - 0.5 * m_thickness) / m_thickness)
                          - (m_biasVoltage - m_depletionVoltage) / m_thickness);

  return E;
}

const ROOT::Math::XYZVector& SensorInfo::getBField(const ROOT::Math::XYZVector& point) const
{
  //  useful just for testing:
  //  static ROOT::Math::XYZVector noBfield(0,0,0);
  //  return noBfield;

  static ROOT::Math::XYZVector oldPoint(0, 0, 1000 * Unit::cm);
  static ROOT::Math::XYZVector oldField(0, 0, 0);
  static double bRadius = 0.5 * Unit::cm;
  if ((point - oldPoint).R() > bRadius) { // renew if far point
    ROOT::Math::XYZVector pointGlobal = pointToGlobal(point, true);
    ROOT::Math::XYZVector bGlobal = BFieldManager::getField(pointGlobal);
    ROOT::Math::XYZVector bLocal = vectorToLocal(bGlobal, true);
    oldPoint = point;
    oldField = bLocal;
  }
  return oldField;
}

const ROOT::Math::XYZVector
SensorInfo::getVelocity(CarrierType carrier, const ROOT::Math::XYZVector& point) const
{
  ROOT::Math::XYZVector E = getEField(point);
  // This is what makes the digitizer slow.
  ROOT::Math::XYZVector B = getBField(point);
  // Set mobility parameters
  double mobility = 0;
  double hallFactor = getHallFactor(carrier);

  if (carrier == electron) {
    mobility = -getElectronMobility(E.R());
  } else {
    mobility = getHoleMobility(E.R());
  }

  double mobilityH = hallFactor * fabs(mobility);

  // Calculate products
  ROOT::Math::XYZVector EcrossB = E.Cross(B);
  ROOT::Math::XYZVector BEdotB = E.Dot(B) * B;
  ROOT::Math::XYZVector velocity = mobility * E + mobility * mobilityH * EcrossB
                                   + mobility * mobilityH * mobilityH * BEdotB;
  velocity *= 1.0 / (1.0 + mobilityH * mobilityH * B.Mag2());
  return velocity;
}

const ROOT::Math::XYZVector& SensorInfo::getLorentzShift(double uCoord, double vCoord) const
{
  static ROOT::Math::XYZVector result;
  double distanceToFrontPlane = 0.5 * m_thickness;
  double distanceToBackPlane = 0.5 * m_thickness;

  // Approximation: calculate drift velocity at the point halfway towards
  // the respective sensor surface.
  ROOT::Math::XYZVector v_e = getVelocity(electron, ROOT::Math::XYZVector(uCoord, vCoord, +0.5 * distanceToFrontPlane));
  ROOT::Math::XYZVector v_h = getVelocity(hole, ROOT::Math::XYZVector(uCoord, vCoord, -0.5 * distanceToBackPlane));

  // Calculate drift directions
  ROOT::Math::XYZVector center_e = fabs(distanceToFrontPlane / v_e.Z()) * v_e;
  ROOT::Math::XYZVector center_h = fabs(distanceToBackPlane / v_h.Z()) * v_h;
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

