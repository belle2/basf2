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
             const float& cotTheta):
  m_tau {d0, phi, omega, z0, cotTheta} {
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
  // named here d0, phi, omega, z0, cotTheta together with an angle
  // chi, can be written:
#if 0
  px = cos(phi + chi) / alpha / omega;
  py = sin(phi + chi) / alpha / omega;
  pz = charge * cotTheta / alpha / omega;
  x =  d0 * sin(phi) + charge / omega * (sin(phi + chi) - sin(phi));
  y = -d0 * cos(phi) + charge / omega * (-cos(phi + chi) + cos(phi));
  z = z0 + charge / omega * cotTheta * chi;
#endif
  const double x = position.X();
  const double y = position.Y();
  const double z = position.Z();

  const double px = momentum.X();
  const double py = momentum.Y();
  const double pz = momentum.Z();

  // We find the perigee parameters by inverting this system of
  // equations and solving for the six variables d0, phi, omega, z0,
  // cotTheta, chi.

  const double ptinv = 1 / hypot(px, py);
  const double omega = charge * ptinv / alpha;
  const double cotTheta = ptinv * pz;

  const double cosphichi = charge * ptinv * px;  // cos(phi + chi)
  const double sinphichi = charge * ptinv * py;  // sin(phi + chi)

  // Helix center in the (x, y) plane:
  const double helX = x + charge * py * alpha;
  const double helY = y - charge * px * alpha;
  const double rhoHel = hypot(helX, helY);

  const double d0 = charge * hypot(helX, helY) - 1 / omega;
  const double phi = atan2(helY, helX) + charge * M_PI / 2;
  const double sinchi = sinphichi * cos(phi) - cosphichi * sin(phi);
  const double chi = asin(sinchi);
  const double z0 = z + charge / omega * cotTheta * chi;

  m_tau.reserve(5);
  m_tau.push_back(d0);
  m_tau.push_back(phi);
  m_tau.push_back(omega);
  m_tau.push_back(z0);
  m_tau.push_back(cotTheta);

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
  return getCotTheta() / (std::fabs(getOmega() * getAlpha(bField)));
}
