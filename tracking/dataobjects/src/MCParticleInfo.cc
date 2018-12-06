/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/MCParticleInfo.h>

using namespace std;
using namespace Belle2;

MCParticleInfo::MCParticleInfo(const MCParticle& the_mcParticle, const TVector3& the_magField)
  : m_mcParticle(the_mcParticle)
  , m_myBz(the_magField.Mag())
  , m_charge(the_mcParticle.getCharge())
{
  // set units to: cm, GeV/c
  m_myBz *= 0.299792 / 100;
} ;

double
MCParticleInfo::getD0()
{

  double pt = this->getPt();
  double px = this->getPx();
  double py = this->getPy();
  double x = this->getX();
  double y = this->getY();
  double R = 1 / this->getOmega(); //cm

  double alpha =  R / pt; //cm/GeV
  double Cx = x + alpha * py; //cm
  double Cy = y - alpha * px; //cm
  double d0 = sqrt(Cx * Cx + Cy * Cy) - R * m_charge; //cm
  d0 = d0 * m_charge;

  return d0;
};

double
MCParticleInfo::getPhi()
{
  return  atan2(this->getPy(), this->getPx()) - this->getChi();
};

double
MCParticleInfo::getChi()
{
  double px = this->getPx();
  double py = this->getPy();
  double pt = this->getPt();
  double x = this->getX();
  double y = this->getY();

  double sinChi = -m_charge * (px * x + py * y);
  double cosChi = -m_charge * (px * y - py * x - (pt * pt) / m_charge / m_myBz);

  return  atan2(sinChi, cosChi);

};

double
MCParticleInfo::getOmega()
{
  double R = this->getPt() / m_myBz / m_charge; // cm
  return 1 / R;
};

double
MCParticleInfo::getZ0()
{
  return this->getZ() + this->getPz() / m_charge / m_myBz * this->getChi();
};

double
MCParticleInfo::getCotTheta()
{
  double tanDip = this->getPz() / this->getPt();

  return tanDip;
};

double
MCParticleInfo::getLambda()
{
  double lambda = atan2(this->getPz(), this->getPt());
  return lambda;
};

