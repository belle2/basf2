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
             const float bZ)
{
  setCartesian(position, momentum, charge, bZ);
}

Helix::Helix(const float& d0,
             const float& phi0,
             const float& omega,
             const float& z0,
             const float& tanLambda)
  : m_d0(d0),
    m_phi0(phi0),
    m_omega(omega),
    m_z0(z0),
    m_tanLambda(tanLambda)
{
}

double Helix::getPerigeeX() const
{
  return getD0() * getSinPhi0();
}

double Helix::getPerigeeY() const
{
  return -getD0() * getCosPhi0();
}

double Helix::getPerigeeZ() const
{
  return getZ0();
}

TVector3 Helix::getPerigee() const
{
  return TVector3(getPerigeeX(), getPerigeeY(), getPerigeeZ());
}

double Helix::getMomentumX(const float bZ) const
{
  return getCosPhi0() * getTransverseMomentum(bZ);
}

double Helix::getMomentumY(const float bZ) const
{
  return getSinPhi0() * getTransverseMomentum(bZ);
}

double Helix::getMomentumZ(const float bZ) const
{
  return getTanLambda() * getTransverseMomentum(bZ);
}

TVector3 Helix:: getMomentum(const float bZ) const
{
  return TVector3(getMomentumX(bZ), getMomentumY(bZ), getMomentumZ(bZ));
}

float Helix::getTransverseMomentum(const float bZ) const
{
  return 1 / std::fabs(getAlpha(bZ) * getOmega());
}

float Helix::getKappa(const float bZ) const
{
  return getOmega() * getAlpha(bZ);
}

double Helix::getAlpha(const float bZ)
{
  return 1.0 / (bZ * TMath::C()) * 1E11;
}

void Helix::reverse()
{
  // All except z0 have to be taken to their opposites
  m_d0 = -m_d0; //d0
  m_phi0 = reversePhi(m_phi0);
  m_omega = -m_omega;
  m_tanLambda = -m_tanLambda;
}

float Helix::getArcLengthAtCylindricalR(const float& cylindricalR) const
{
  // Slight trick here
  // Since the sought point is on the helix we treat it as the perigee
  // and the origin as the point to extrapolate to.
  // We know the distance of the origin to the circle, which is just d0
  // The direct distance from the origin to the imaginary perigee is just the given cylindricalR.
  const float dr = getD0();
  const float deltaCylindricalR = cylindricalR;
  const double absArcLength = calcArcLengthAtDeltaCylindricalRAndDr(deltaCylindricalR, dr);
  return absArcLength;
}

float Helix::getArcLengthAtXY(const float& x, const float& y) const
{
  double dr = 0;
  double arcLength = 0;
  calcArcLengthAndDrAtXY(x, y, arcLength, dr);
  return arcLength;
}

TVector3 Helix::getPositionAtArcLength(const float& arcLength) const
{
  /*
    /   \     /                      \     /                             \
    | x |     | cos phi0   -sin phi0 |     |    - sin(chi)  / omega      |
    |   |  =  |                      |  *  |                             |
    | y |     | sin phi0    cos phi0 |     | (1 - cos(chi)) / omega - d0 |
    \   /     \                      /     \                             /

    and

    z = tanLambda * arclength + z0;

    where chi = -arcLength * omega

    Here arcLength means the arc length of the circle in the xy projection
    traversed in the forward direction.

    // Old definition identical?
    x =  d0 * sin(phi0) + charge / omega * (sin(phi0 + chi) - sin(phi0));
    y = -d0 * cos(phi0) + charge / omega * (-cos(phi0 + chi) + cos(phi0));
    // Actually no - have to talk back to Markus about this.
  */

  // First calculating the position assuming the circle center lies on the y axes (phi0 = 0)
  // Rotate to the right phi position afterwards
  // Using the sinus cardinalis yields expressions that are smooth in the limit of omega -> 0

  // Do calculations in double
  const double chi = -arcLength * getOmega();
  const double chiHalf = chi / 2.0;

  using boost::math::sinc_pi;

  const double x = arcLength * sinc_pi(chi);
  const double y = arcLength * sinc_pi(chiHalf) * sin(chiHalf) - getD0();

  // const double z = s * tan lambda + z0
  const double z = fma((double)arcLength, getTanLambda(),  getZ0());

  // Unrotated position
  TVector3 position(x, y, z);

  // Rotate to the right phi0 position
  position.RotateZ(getPhi0());

  return position;
}

TVector3 Helix::getTangentialAtArcLength(const float& arcLength) const
{
  const double omega = getOmega();
  const double phi0 = getPhi0();
  const double tanLambda = getTanLambda();

  const double chi = - omega * arcLength;

  const double tx = cos(chi + phi0);
  const double ty = sin(chi + phi0);
  const double tz = tanLambda;

  TVector3 tangential(tx, ty, tz);
  return tangential;
}


TVector3 Helix::getUnitTangentialAtArcLength(const float& arcLength) const
{
  TVector3 unitTangential = getTangentialAtArcLength(arcLength);
  const double norm = hypot(1, getTanLambda());
  const double invNorm = 1 / norm;
  unitTangential *= invNorm;
  return unitTangential;
}

TVector3 Helix::getMomentumAtArcLength(const float& arcLength, const float& bz) const
{
  TVector3 momentum = getTangentialAtArcLength(arcLength);
  const double pr = getTransverseMomentum(bz);
  momentum *= pr;

  return momentum;
}

float Helix::passiveMoveBy(const TVector3& by)
{
  // First calculate the distance of the new origin to the helix in the xy projection
  double new_d0 = 0;
  double arcLength = 0;
  calcArcLengthAndDrAtXY(by.X(), by.Y(), arcLength, new_d0);

  // Third the new phi0 and z0 can be calculated from the arc length
  double chi = -arcLength * getOmega();
  double new_phi0 = m_phi0 + chi;
  double new_z0 = getZ0() - by.Z() + getTanLambda() * arcLength;

  /// Update the parameters inplace. Omega and tan lambda are unchanged
  m_d0 = new_d0;
  m_phi0 = new_phi0;
  m_z0 = new_z0;

  return arcLength;
}

double Helix::calcArcLengthFromSecantLength(const double& secantLength) const
{
  return secantLength * calcSecantLengthToArcLengthFactor(secantLength);
}


double Helix::calcSecantLengthToArcLengthFactor(const double& secantLength) const
{
  double x = secantLength * getOmega() / 2.0;
  return calcASinXDividedByX(x);
}

double Helix::calcASinXDividedByX(const double& x)
{
  // Approximation of asin(x) / x
  // Inspired by BOOST's sinc

  BOOST_MATH_STD_USING;

  double const taylor_n_bound = boost::math::tools::forth_root_epsilon<double>();

  if (abs(x) >= taylor_n_bound) {
    if (fabs(x) == 1) {
      return  M_PI / 2.0;

    } else {
      return asin(x) / x;

    }

  } else {
    // approximation by taylor series in x at 0 up to order 0
    double result = 1.0;

    double const taylor_0_bound = boost::math::tools::epsilon<double>();
    if (abs(x) >= taylor_0_bound) {
      double x2 = x * x;
      // approximation by taylor series in x at 0 up to order 2
      result += x2 / 6.0;

      double const taylor_2_bound = boost::math::tools::root_epsilon<double>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        result += x2 * x2 * (3.0 / 40.0);
      }
    }
    return result;
  }

}

void Helix::calcArcLengthAndDrAtXY(const float& x, const float& y, double& arcLength, double& dr) const
{
  // Prepare common variables
  const double omega = getOmega();
  const double cosPhi0 = getCosPhi0();
  const double sinPhi0 = getSinPhi0();

  const double perigeeX = getPerigeeX();
  const double perigeeY = getPerigeeY();

  const double deltaX = x - perigeeX;
  const double deltaY = y - perigeeY;

  const double deltaParallel = deltaX * cosPhi0 + deltaY * sinPhi0;
  const double deltaOrthogonal = deltaY * cosPhi0 - deltaX * sinPhi0;
  const double deltaCylindricalR = hypot(deltaX, deltaY);
  const double deltaCylindricalRSquared = deltaCylindricalR * deltaCylindricalR;

  // Calculate dr
  const double A = 2 * deltaOrthogonal + omega * deltaCylindricalRSquared;
  const double U = sqrt(1 + omega * A);
  dr = A / (1 + U);

  // Calculate the absolute value of the arc length
  const double absArcLength = calcArcLengthAtDeltaCylindricalRAndDr(deltaCylindricalR, dr);

  // Fixing the sign looking if the new origin lies in the forward direction in the xy projection.
  arcLength = std::copysign(absArcLength, deltaParallel);
}

double Helix::calcArcLengthAtDeltaCylindricalRAndDr(const double& deltaCylindricalR, const double& dr) const
{
  const double omega = getOmega();

  // When cylindricalR and d0 are approximatly the same and the resulting difference of the squares turns out
  // to be -10^-18 in double precision, which makes the sqrt return nan.
  // Do calculation in single precision here to negating these rounding errors of order -10^-18.
  float deltaCylindricalRSquared = deltaCylindricalR * deltaCylindricalR;
  float drSquared = dr * dr;
  double secantLength = sqrt((deltaCylindricalRSquared - drSquared) / (1 + dr * omega));
  // Note : The above line results from simplifications for the normal n_0, n_1, n_2, n_3 representation
  // It might have an geometrical interpretation. If you have one please insert it here.

  return calcArcLengthFromSecantLength(secantLength);
}

void Helix::setCartesian(const TVector3& position,
                         const TVector3& momentum,
                         const short int charge,
                         const float bZ)
{
  const double alpha = getAlpha(bZ);

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

  // Helix center in the (x, y) plane:
  const double helX = x + charge * py * alpha;
  const double helY = y - charge * px * alpha;
  //const double rhoHel = hypot(helX, helY);

  const double d0 = charge * hypot(helX, helY) - 1 / omega;

  double phi0 = atan2(helY, helX) + charge * M_PI / 2;

  // Bring phi0 to the interval [-pi, pi]
  phi0 = remainder(phi0, 2 * M_PI);

  const double cosPhi0 = cos(phi0);
  const double sinPhi0 = sin(phi0);

  const double cosphi0chi = ptinv * px;  // cos(phi0 + chi)
  const double sinphi0chi = ptinv * py;  // sin(phi0 + chi)

  const double coschi =    cosphi0chi * cosPhi0 + sinphi0chi * sinPhi0;
  const double sinchi =  - cosphi0chi * sinPhi0 + sinphi0chi * cosPhi0;

  const double chi = atan2(sinchi, coschi);

  const double arcLength = -chi / omega;
  const double z0 = z - arcLength * tanLambda;

  m_omega = omega;
  m_phi0 = phi0;
  m_d0 = d0;
  m_tanLambda = tanLambda;
  m_z0 = z0;

}

namespace Belle2 {
  std::ostream& operator<<(std::ostream& output, const Helix& helix)
  {
    return output
           << "Helix("
           << "d0=" << helix.getD0() << ", "
           << "phi0=" << helix.getPhi0() << ", "
           << "omega=" << helix.getOmega() << ", "
           << "z0=" << helix.getZ0() << ", "
           << "tanLambda=" << helix.getTanLambda() << ")";
  }
}
