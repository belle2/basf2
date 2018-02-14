/*
 * SensorInfo.cc
 *
 *  Created on: Dec 19, 2013
 *      Author: kvasnicka
 */

#include <pxd/geometry/SensorInfo.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/geometry/BFieldManager.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;


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

const TVector3 SensorInfo::getEField(const TVector3& point) const
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

const TVector3 SensorInfo::getBField(const TVector3& point) const
{
  TVector3 pointGlobal = pointToGlobal(point);
  TVector3 bGlobal = BFieldManager::getField(pointGlobal);
  TVector3 bLocal = vectorToLocal(bGlobal);
  return bLocal;
}

const TVector3 SensorInfo::getDriftVelocity(const TVector3& E,
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

int SensorInfo::getPixelKind(const VxdID sensorID, double v) const
{
  const SensorInfo& Info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
  int i_pixelKind = Info.getVPitchID(v);
  if (Info.getID().getLayerNumber() == 2) i_pixelKind += 4;
  if (Info.getID().getSensorNumber() == 2) i_pixelKind += 2;
  return i_pixelKind;
}

int SensorInfo::getPixelKindNew(const VxdID& sensorID, int vID) const
{
  const SensorInfo& Info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(sensorID));
  double v = Info.getVCellPosition(vID);
  double vPitch = Info.getVPitch(v);
  int  i_pixelKind = 0;

  if (std::fabs(vPitch - 0.0055) < 0.0001)
    i_pixelKind = 0;
  else if (std::fabs(vPitch - 0.0060) < 0.0001)
    i_pixelKind = 1;
  else if (std::fabs(vPitch - 0.0070) < 0.0001)
    i_pixelKind = 2;
  else if (std::fabs(vPitch - 0.0085) < 0.0001)
    i_pixelKind = 3;
  else
    B2FATAL("Unexpected pixel vPitch.");
  return i_pixelKind;
}


const TVector3 SensorInfo::getLorentzShift(double u, double v) const
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
    TVector3 velocity = getDriftVelocity(currentEField, currentBField);
    position += weightGL[iz] / velocity.Z() * velocity;
  } // for knots
  position.SetZ(0);
  position.SetX(position.X() - u);
  position.SetY(position.Y() - v);
  return position;
}

void SensorInfo::cook()
{
  m_iup = m_uCells / m_width;
  m_up = m_width / m_uCells;

  m_vsplit = m_length * (m_splitLength - 0.5);

  m_vp = m_length * m_splitLength / m_vCells;
  m_ivp = 1 / m_vp;

  m_vp2 = m_length * (1 - m_splitLength) / m_vCells2;
  m_ivp2 = 1 / m_vp2;

  m_hxIG = 0.5 * m_up  - m_clearBorderSmallPitch;
  m_mIGL = 0.5 * (m_vp - m_sourceBorderLargePitch + m_drainBorderLargePitch);
  m_sIGL = 0.5 * (m_vp - m_sourceBorderLargePitch - m_drainBorderLargePitch);
  m_mIGS = 0.5 * (m_vp2 - m_sourceBorderSmallPitch + m_drainBorderSmallPitch);
  m_sIGS = 0.5 * (m_vp2 - m_sourceBorderSmallPitch - m_drainBorderSmallPitch);
}

int SensorInfo::getTrappedID(double x, double y) const
{
  double huCells = 0.5 * m_uCells;
  double ix = floor(x * m_iup + huCells);
  int jx = ix;
  double x0 = (ix + 0.5 - huCells) * m_up;

  if (fabs(x - x0) < m_hxIG) {
    if ((unsigned)jx >= (unsigned)m_uCells) return -1;
    double ys = y - m_vsplit;
    if (ys >= 0) {
      double iy = floor(ys * m_ivp2);
      int jy = iy;
      iy = jy / 2;
      double y0 = iy * m_vp2 * 2 + m_vp2;
      double yl = fabs(ys - y0);
      if (fabs(yl - m_mIGS) < m_sIGS) {
        if ((unsigned)jy >= (unsigned)m_vCells2) return -1;
        return jx + m_uCells * (jy + m_vCells);
      }
      return -1;
    } else {
      ys = y + 0.5 * m_length;
      double iy = floor(ys * m_ivp);
      int jy = iy;
      iy = jy / 2;
      double y0 = iy * m_vp * 2 + m_vp;
      double yl = fabs(ys - y0);
      if (fabs(yl - m_mIGL) < m_sIGL) {
        if ((unsigned)jy >= (unsigned)m_vCells) return -1;
        return jx + m_uCells * jy;
      }
      return -1;
    }
  }
  return -1;
}
