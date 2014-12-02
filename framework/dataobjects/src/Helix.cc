/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/Helix.h>
#include <TMath.h>

#include <framework/logging/Logger.h>

#include <boost/math/special_functions/sinc.hpp>
#include <boost/math/tools/precision.hpp>

using namespace Belle2;

ClassImp(Helix);

Helix::Helix()
{
}

Helix::Helix(const TVector3& position,
             const TVector3& momentum,
             const short int charge,
             const float bField)
{
  cartesianToPerigee(position, momentum, charge, bField);
}


Helix::Helix(const float& d0,
             const float& phi,
             const float& omega,
             const float& z0,
             const float& tanLambda):
  m_tau {d0, phi, omega, z0, tanLambda} {
}



Helix::Helix(const std::vector<float>& tau) :
  m_tau(tau)
{
}

// This class should be able to give back Helix information either in Perigee Parametrisation
// or as starting position + momentum.
TVector3 Helix::getPosition() const
{
  return TVector3(calcXFromPerigee(), calcYFromPerigee(), calcZFromPerigee());
}

TVector3 Helix::getMomentum(const float bField) const
{
  return TVector3(calcPxFromPerigee(bField), calcPyFromPerigee(bField), calcPzFromPerigee(bField));
}

float Helix::getTransverseMomentum(const float bField) const
{
  return std::fabs(1 / getAlpha(bField) / m_tau.at(2));
}

void Helix::reverse()
{
  // All except z0 have to be taken to their opposites
  m_tau.at(0) = -m_tau.at(0); //d0
  m_tau.at(1) = m_tau.at(1) < 0 ? m_tau.at(1) + M_PI : m_tau.at(1) - M_PI; //phi
  m_tau.at(2) = -m_tau.at(2); //omega
  m_tau.at(4) = -m_tau.at(4); //coTheta
}

TVector3 Helix::getPositionAtArcLength(const float& arcLength) const
{
  /*
  x =  d0 * sin(phi) + charge / omega * (sin(phi + chi) - sin(phi));
  y = -d0 * cos(phi) + charge / omega * (-cos(phi + chi) + cos(phi));
  z = z0 + charge / omega * tanLambda * chi;

  where chi = arcLength * omega
  */

  // First calculating the position assuming the circle center to lies on the y axes (assuming phi0 = 0)
  // Rotate to the right phi position afterwards
  // Using the sinus cardinalis yields expressions that are smooth in the limit of omega -> 0
  const float x = arcLength * sinc((double)arcLength * getOmega());
  const float y = -arcLength * cosc((double)arcLength * getOmega()) - getD0();

  // z = s * tan lambda  + z0
  const float z = fma((double)arcLength, getTanLambda(),  getZ0());

  // Unrotated position
  TVector3 position(x, y, z);

  // Rotate to the right phi position
  const float rotatePhi = getPhi();
  position.RotateZ(rotatePhi);

  return position;
}

float Helix::getArcLengthAtPolarR(const float& polarR) const
{
  double d0 = getD0();
  double omega = getOmega();
  double secantLength = sqrt(((double)polarR * polarR  - d0 * d0) / (1 + d0 * omega));
  return calcArcLengthFromSecantLength(secantLength);
}

double Helix::sinc(const double& x)
{
  return boost::math::sinc_pi(x);
}

double Helix::cosc(const double& x)
{
  // Though fundamentally appealing, since it is the complex adjoint of the sinus cardinalis
  // there is no standard implementation of this function, which is why we draw inspiration
  // from the boost sinc_pi function and modify it.

  BOOST_MATH_STD_USING;

  double const taylor_n_bound = boost::math::tools::forth_root_epsilon<double>();

  if (abs(x) >= taylor_n_bound) {
    return (1 - cos(x)) / x;

  } else {
    // approximation by taylor series in x at 0 up to order 1
    double result = x / 2.0;

    double const taylor_3_bound = boost::math::tools::epsilon<double>();
    if (abs(x) >= taylor_3_bound) {
      // approximation by taylor series in x at 0 up to order 3
      double const x2 = x * x;
      double const x3 = x2 * x;
      result -= x3 / 24.0;

      double const taylor_5_bound = boost::math::tools::root_epsilon<double>();
      if (abs(x) >= taylor_5_bound) {
        // approximation by taylor series in x at 0 up to order 5
        double const x5 = x2 * x3;
        result += x5 / 720;

      }
    }
    return result;
  }
}

double Helix::calcArcLengthFromSecantLength(const double& secantLength) const
{
  double x = secantLength * getOmega() / 2.0;

  // Need asin(x) / x also for low values
  // Use approximation inspired by BOOST's sinc
  BOOST_MATH_STD_USING;

  double const taylor_n_bound = boost::math::tools::forth_root_epsilon<double>();

  if (abs(x) >= taylor_n_bound) {
    if (fabs(x) == 1) {
      return secantLength * M_PI / 2.0;

    } else {
      return 2.0 * asin(x) / getOmega();

    }

  } else {
    // Approximation of asin(x) / x
    // Inspired by BOOST's sinc

    // approximation by taylor series in x at 0 up to order 0
    double secantLengthFactor = 1.0;

    double const taylor_0_bound = boost::math::tools::epsilon<double>();
    if (abs(x) >= taylor_0_bound) {
      double x2 = x * x;
      // approximation by taylor series in x at 0 up to order 2
      secantLengthFactor += x2 / 6.0;

      double const taylor_2_bound = boost::math::tools::root_epsilon<double>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        secantLengthFactor += x2 * x2 * (3.0 / 40.0);
      }
    }
    return  secantLengthFactor * secantLength;
  }

}

void Helix::cartesianToPerigee(const TVector3& position,
                               const TVector3& momentum,
                               const short int charge,
                               const float bField)
{
  const double alpha = getAlpha(bField);

  // We allow for the case that position, momentum are not given
  // exactly in the perigee.  Therefore we have to solve the
  // equations.  Any point on the helix, using the perigee parameters
  // as in "Fast vertex fitting with a local parametrization of tracks
  // - Billoir, Pierre et al. Nucl.Instrum.Meth. A311 (1992) 139-150"
  // named here d0, phi, omega, z0, tanLambda together with an angle
  // chi, can be written:
#if 0
  px = cos(phi + chi) / alpha / omega;
  py = sin(phi + chi) / alpha / omega;
  pz = charge * tanLambda / alpha / omega;
  x =  d0 * sin(phi) + charge / omega * (sin(phi + chi) - sin(phi));
  y = -d0 * cos(phi) + charge / omega * (-cos(phi + chi) + cos(phi));
  z = z0 + charge / omega * tanLambda * chi;
#endif
  const double x = position.X();
  const double y = position.Y();
  const double z = position.Z();

  const double px = momentum.X();
  const double py = momentum.Y();
  const double pz = momentum.Z();

  // We find the perigee parameters by inverting this system of
  // equations and solving for the six variables d0, phi, omega, z0,
  // tanLambda, chi.

  const double ptinv = 1 / hypot(px, py);
  const double omega = charge * ptinv / alpha;
  const double tanLambda = ptinv * pz;

  const double cosphichi = charge * ptinv * px;  // cos(phi + chi)
  const double sinphichi = charge * ptinv * py;  // sin(phi + chi)

  // Helix center in the (x, y) plane:
  const double helX = x + charge * py * alpha;
  const double helY = y - charge * px * alpha;
  //const double rhoHel = hypot(helX, helY);

  const double d0 = charge * hypot(helX, helY) - 1 / omega;
  const double phi = atan2(helY, helX) + charge * M_PI / 2;
  const double sinchi = sinphichi * cos(phi) - cosphichi * sin(phi);
  const double chi = asin(sinchi);
  const double z0 = z + charge / omega * tanLambda * chi;

  m_tau.reserve(5);
  m_tau.push_back(d0);
  m_tau.push_back(phi);
  m_tau.push_back(omega);
  m_tau.push_back(z0);
  m_tau.push_back(tanLambda);

}

double Helix::getAlpha(const float bField) const
{
  return 1.0 / (bField * TMath::C()) * 1E11;
}

double Helix::calcXFromPerigee() const
{
  return getD0() * std::sin((double)getPhi());
}

double Helix::calcYFromPerigee() const
{
  return -getD0() * std::cos((double)getPhi());
}

double Helix::calcZFromPerigee() const
{
  return getZ0();
}

double Helix::calcPxFromPerigee(const float bField) const
{
  return std::cos((double)getPhi()) / (std::fabs(getOmega() * getAlpha(bField)));
}

double Helix::calcPyFromPerigee(const float bField) const
{
  return std::sin((double)getPhi()) / (std::fabs(getOmega() * getAlpha(bField)));
}

double Helix::calcPzFromPerigee(const float bField) const
{
  return getTanLambda() / (std::fabs(getOmega() * getAlpha(bField)));
}
