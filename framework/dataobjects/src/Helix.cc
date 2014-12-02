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



Helix::Helix(const float& omega,
             const float& phi0,
             const float& d0,
             const float& tanLambda,
             const float& z0)
  : m_omega(omega),
    m_phi0(phi0),
    m_d0(d0),
    m_tanLambda(tanLambda),
    m_z0(z0)
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
  return std::fabs(1 / getAlpha(bField) / getOmega());
}

void Helix::reverse()
{
  // All except z0 have to be taken to their opposites
  m_d0 = -m_d0; //d0
  m_phi0 = reversePhi(m_phi0);
  m_omega = -m_omega;
  m_tanLambda = -m_tanLambda;
}

float Helix::getArcLengthAtPolarR(const float& polarR) const
{
  double d0 = getD0();
  double omega = getOmega();
  double secantLength = sqrt(((double)polarR * polarR  - d0 * d0) / (1 + d0 * omega));
  return calcArcLengthFromSecantLength(secantLength);
}

TVector3 Helix::getPositionAtArcLength(const float& arcLength) const
{
  /*
    /   \     /                      \     /                             \
    | x |     | cos phi0   -sin phi0 |     |      sin(chi)  / omega      |
    |   |  =  |                      |  *  |                             |
    | y |     | sin phi0    cos phi0 |     | -(1 - cos(chi)) / omega - d0 |
    \   /     \                      /     \                             /

    and

    z = z0 + charge / omega * tanLambda * chi;

    where chi = arcLength * omega

    // Old definitionb identical?
    x =  d0 * sin(phi0) + charge / omega * (sin(phi0 + chi) - sin(phi0));
    y = -d0 * cos(phi0) + charge / omega * (-cos(phi0 + chi) + cos(phi0));
  */

  // First calculating the position assuming the circle center to lies on the y axes (assuming phi0 = 0)
  // Rotate to the right phi position afterwards
  // Using the sinus cardinalis yields expressions that are smooth in the limit of omega -> 0

  // Do calculations in double
  const double chi = arcLength * getOmega();
  const double chiHalf = chi / 2.0;

  using boost::math::sinc_pi;

  const double x = arcLength * sinc_pi(chi);
  const double y = -arcLength * sinc_pi(chiHalf) * sin(chiHalf) - getD0();

  // const double z = s * tan lambda + z0
  const double z = fma((double)arcLength, getTanLambda(),  getZ0());

  // Unrotated position
  TVector3 position(x, y, z);

  // Rotate to the right phi position
  position.RotateZ(getPhi0());

  return position;
}


TVector3 Helix::getUnitTangentialAtArcLength(const float& arcLength) const
{

  const double omega = getOmega();
  const double phi0 = getPhi0();
  const double tanLambda = getTanLambda();

  const double norm = hypot(1, tanLambda);
  const double invNorm = 1 / norm;

  const double tx = cos(arcLength * omega + phi0) * invNorm;
  const double ty = sin(arcLength * omega + phi0) * invNorm;
  const double tz = tanLambda * invNorm;

  return TVector3(tx, ty, tz);
}



TVector3 Helix::getMomentumAtArcLength(const float& arcLength, const float& bz) const
{
  const double omega = getOmega();
  const double phi0 = getPhi0();
  const double tanLambda = getTanLambda();

  // For the straight line case we return a unit length vector
  const double pr = omega == 0 or bz == 0 ? 1 / hypot(1, tanLambda) : getTransverseMomentum(bz);

  const double px = cos(arcLength * omega + phi0) * pr;
  const double py = sin(arcLength * omega + phi0) * pr;
  const double pz = tanLambda * pr;


  // const double px = std::cos((double)getPhi0()) / (std::fabs(getOmega() * getAlpha(bz)));
  // const double py = std::sin((double)getPhi0()) / (std::fabs(getOmega() * getAlpha(bz)));
  // const double pz = getTanLambda() / (std::fabs(getOmega() * getAlpha(bz)));

  // const double px = calcPxFromPerigee(bz);
  // const double py = calcPyFromPerigee(bz);
  // const double pz = calcPzFromPerigee(bz);

  TVector3 momentum(px, py, pz);
  //momentum.RotateZ(arcLength * omega);

  return momentum;
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
  // named here omega, phi0, d0, tanLambda, z0 together with an angle
  // chi, can be written:
#if 0
  px = cos(phi0 + chi) / alpha / omega;
  py = sin(phi0 + chi) / alpha / omega;
  pz = charge * tanLambda / alpha / omega;
  x =  d0 * sin(phi0) + charge / omega * (sin(phi0 + chi) - sin(phi0));
  y = -d0 * cos(phi0) + charge / omega * (-cos(phi0 + chi) + cos(phi0));
  z = z0 + charge / omega * tanLambda * chi;
#endif
  const double x = position.X();
  const double y = position.Y();
  const double z = position.Z();

  const double px = momentum.X();
  const double py = momentum.Y();
  const double pz = momentum.Z();

  // We find the perigee parameters by inverting this system of
  // equations and solving for the six variables d0, phi0, omega, z0,
  // tanLambda, chi.

  const double ptinv = 1 / hypot(px, py);
  const double omega = charge * ptinv / alpha;
  const double tanLambda = ptinv * pz;

  const double cosphi0chi = charge * ptinv * px;  // cos(phi0 + chi)
  const double sinphi0chi = charge * ptinv * py;  // sin(phi0 + chi)

  // Helix center in the (x, y) plane:
  const double helX = x + charge * py * alpha;
  const double helY = y - charge * px * alpha;
  //const double rhoHel = hypot(helX, helY);

  const double d0 = charge * hypot(helX, helY) - 1 / omega;

  double phi0 = atan2(helY, helX) + charge * M_PI / 2;
  // Bring phi0 to the interval [-pi, pi]
  phi0 = remainder(phi0, 2 * M_PI);

  const double sinchi = sinphi0chi * cos(phi0) - cosphi0chi * sin(phi0);
  const double chi = asin(sinchi);
  const double z0 = z + charge / omega * tanLambda * chi;

  // Bring phi0 to the interval [-pi, pi]
  phi0 = fmod(phi0, 2 * M_PI);
  if ((phi0 < -M_PI) or (phi0 > M_PI)) {
    phi0 = reversePhi(phi0);
  }

  m_omega = omega;
  m_phi0 = phi0;
  m_d0 = d0;
  m_tanLambda = tanLambda;
  m_z0 = z0;

}

double Helix::getAlpha(const float bField) const
{
  return 1.0 / (bField * TMath::C()) * 1E11;
}

double Helix::calcXFromPerigee() const
{
  return getD0() * std::sin((double)getPhi0());
}

double Helix::calcYFromPerigee() const
{
  return -getD0() * std::cos((double)getPhi0());
}

double Helix::calcZFromPerigee() const
{
  return getZ0();
}

double Helix::calcPxFromPerigee(const float bField) const
{
  return std::cos((double)getPhi0()) / (std::fabs(getOmega() * getAlpha(bField)));
}

double Helix::calcPyFromPerigee(const float bField) const
{
  return std::sin((double)getPhi0()) / (std::fabs(getOmega() * getAlpha(bField)));
}

double Helix::calcPzFromPerigee(const float bField) const
{
  return getTanLambda() / (std::fabs(getOmega() * getAlpha(bField)));
}

namespace Belle2 {
  std::ostream& operator<<(std::ostream& output, const Helix& helix)
  {
    return output
           << "Helix("
           << "omega=" << helix.getOmega() << ", "
           << "phi0=" << helix.getPhi0() << ", "
           << "d0=" << helix.getD0() << ", "
           << "tanLambda=" << helix.getTanLambda() << ", "
           << "z0=" << helix.getZ0() << ")";
  }
}
