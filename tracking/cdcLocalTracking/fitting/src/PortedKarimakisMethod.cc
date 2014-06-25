/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PortedKarimakisMethod.h"

#include <Eigen/Dense>

#include <tracking/cdcLocalTracking/geometry/UncertainPerigeeCircle.h>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(PortedKarimakisMethod)


PortedKarimakisMethod::PortedKarimakisMethod() :
  _curved(true),
  _npar(_curved ? 3 : 2),
  _parameters(_npar),
  _covariance(_npar)
{
}



PortedKarimakisMethod::~PortedKarimakisMethod()
{
}



void PortedKarimakisMethod::update(CDCTrajectory2D& trajectory2D,
                                   CDCObservations2D& observations2D) const
{

  size_t nObservations = observations2D.size();
  trajectory2D.clear();
  if (not nObservations) return;
  simpleFitXY(true);

  FloatType xRef = observations2D.getX(0);
  FloatType yRef = observations2D.getY(0);

  //Vector2D ref(xRef, yRef);
  //Vector2D ref = Vector2D(0.0, 0.0);
  Vector2D ref = observations2D.getCentralPoint();
  //Vector2D ref = Vector2D(0.5, 0.0);
  B2INFO("Reference point " << ref);

  observations2D.centralize(ref);

  for (size_t iObservation = 0; iObservation < nObservations; ++iObservation) {
    FloatType x = observations2D.getX(iObservation);
    FloatType y = observations2D.getY(iObservation);
    addPoint(x, y, 1.0);
  }

  double Chi2 = 0;
  int nPoints = 0;

  fit(Chi2, nPoints);

  FloatType curvature = _parameters(0);
  FloatType tangentialPhi = _parameters(1) + (_parameters(1) > 0 ? - PI : + PI);
  tangentialPhi = _parameters(1);
  FloatType impact = _parameters(2);

  UncertainPerigeeCircle perigeeCircle(curvature, tangentialPhi, impact, _covariance);

  FloatType frontX = observations2D.getX(0);
  FloatType frontY = observations2D.getY(0);
  Vector2D frontPos(frontX, frontY);

  FloatType backX = observations2D.getX(nObservations - 1);
  FloatType backY = observations2D.getY(nObservations - 1);
  Vector2D backPos(backX, backY);

  FloatType totalPerps = perigeeCircle.lengthOnCurve(frontPos, backPos);
  if (totalPerps < 0) perigeeCircle.reverse();

  perigeeCircle.passiveMoveBy(-ref);
  trajectory2D.setCircle(perigeeCircle);

}

void PortedKarimakisMethod::simpleFitXY() const
{
  _numPoints = 0;
  _sx = _sy = _sxx = _sxy = _syy = _sw = 0.;
  _sr = _sxr = _syr = _srr = 0.;
}


void PortedKarimakisMethod::addPoint(double x, double y, double w) const
{
  _numPoints++;
  double xl = x;
  double yl = y;
  _sw += w;
  _sx += w * xl;
  _sy += w * yl;
  _sxx += w * xl * xl;
  _sxy += w * xl * yl;
  _syy += w * yl * yl;
  if (_curved) {
    double r2 = xl * xl + yl * yl;
    _sr += w * r2;
    _sxr += w * r2 * xl;
    _syr += w * r2 * yl;
    _srr += w * r2 * r2;
  }
}


int PortedKarimakisMethod::fit(double& Chi2, int& nPoints) const
{
  // averages
  double ax = _sx / _sw;
  double ay = _sy / _sw;
  double ar = _sr / _sw;
  double axx = _sxx / _sw;
  double ayy = _syy / _sw;
  double axy = _sxy / _sw;
  double axr = _sxr / _sw;
  double ayr = _syr / _sw;
  double arr = _srr / _sw;
  // variances
  double cxx = axx - ax * ax;
  double cyy = ayy - ay * ay;
  double cxy = axy - ax * ay;
  double cxr = axr - ax * ar;
  double cyr = ayr - ay * ar;
  double crr = arr - ar * ar;

  double q1, q2;
  if (_curved) {
    q1 = crr * cxy - cxr * cyr;
    q2 = crr * (cxx - cyy) - cxr * cxr + cyr * cyr;
  } else {
    q1 = cxy;
    q2 = cxx - cyy;
  }
  double phi = 0.5 * atan2(2. * q1, q2);
  double sinphi = sin(phi);
  double cosphi = cos(phi);

  // compare phi with initial track direction
  //B2INFO(ax + _xRef);
  //B2INFO(ay + _yRef);
  //B2INFO(cosphi);
  //B2INFO(sinphi);

  //if (cosphi * (ax + _xRef) + sinphi * (ay + _yRef) < 0.) {
  if ((cosphi * ax + sinphi * ay) < 0.) {
    // reverse direction
    //B2INFO("Reversed phi");
    phi -= (phi > 0.) ? M_PI : -M_PI;
    cosphi = -cosphi;
    sinphi = -sinphi;
  } else {
    //B2INFO("Unreversed phi");
  }

  if (_curved) {
    double kappa = (sinphi * cxr - cosphi * cyr) / crr;
    double delta = -kappa * ar + sinphi * ax - cosphi * ay;
    // track parameters
    double rho = -2. * kappa / sqrt(1. - 4. * delta * kappa);
    double d = 2. * delta / (1. + sqrt(1. - 4. * delta * kappa));
    _parameters[0] = rho;
    _parameters[1] = phi;
    _parameters[2] = d;
    // chi2
    double u = 1. - rho * d;
    Chi2 = _sw * u * u * (sinphi * sinphi * cxx - 2. * sinphi * cosphi * cxy + cosphi * cosphi * cyy - kappa * kappa * crr);
    //cout << " xyfit " << Chi2 << " " << _numPoints << " " <<_npar << ": " << rho << " " << phi << " " << d << endl;
    // calculate covariance matrix
    double sa = sinphi * _sx - cosphi * _sy;
    double sb = cosphi * _sx + sinphi * _sy;
    double sc = (sinphi * sinphi - cosphi * cosphi) * _sxy + sinphi * cosphi * (_sxx - _syy);
    double sd = sinphi * _sxr - cosphi * _syr;
    double saa = sinphi * sinphi * _sxx - 2. * sinphi * cosphi * _sxy + cosphi * cosphi * _syy;
    _covariance[0][0] = 0.25 * _srr - d * (sd - d * (saa + 0.5 * _sr - d * (sa - 0.25 * d * _sw)));
    _covariance[0][1] = u * (0.5 * (cosphi * _sxr + sinphi * _syr) - d * (sc - 0.5 * d * sb));
    _covariance[1][0] = _covariance[0][1];
    _covariance[1][1] = u * u * (cosphi * cosphi * _sxx + 2. * cosphi * sinphi * _sxy + sinphi * sinphi * _syy);
    _covariance[0][2] = rho * (-0.5 * sd + d * saa) - 0.5 * u * _sr + 0.5 * d * ((3 * u - 1.) * sa - u * d * _sw);
    _covariance[2][0] = _covariance[0][2];
    _covariance[1][2] = -u * (rho * sc + u * sb);
    _covariance[2][1] = _covariance[1][2];
    _covariance[2][2] = rho * (rho * saa + 2 * u * sa) + u * u * _sw;
  } else {
    // track parameters
    double d = sinphi * ax - cosphi * ay;
    _parameters[0] = phi;
    _parameters[1] = d;
    // chi2
    Chi2 = _sw * (sinphi * sinphi * cxx - 2. * sinphi * cosphi * cxy + cosphi * cosphi * cyy);
    //cout << " xyfit " << chi2 << " " << _numPoints-_npar << ": " << phi << " " << d << endl;
    // calculate covariance matrix
    _covariance[0][0] = cosphi * cosphi * _sxx + 2. * cosphi * sinphi * _sxy + sinphi * sinphi * _syy;
    _covariance[0][1] = -(cosphi * _sx + sinphi * _sy);
    _covariance[1][0] = _covariance[0][1];
    _covariance[1][1] = _sw;
  }
  _covariance.Invert();
  nPoints = _numPoints;
  return _npar;
}


TVectorD PortedKarimakisMethod::getPar() const
{
  return _parameters;
}


TMatrixDSym PortedKarimakisMethod::getCov() const
{
  return _covariance;
}
