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
#include <tracking/modules/trackingPerformanceEvaluation/MCParticleInfo.h>

using namespace std;
using namespace Belle2;

MCParticleInfo::MCParticleInfo(const MCParticle& the_mcParticle, const TVector3& the_magField)
  : m_mcParticle(the_mcParticle)
  , m_magField(the_magField)
  , m_charge(the_mcParticle.getCharge())
{
} ;

MCParticleInfo::~MCParticleInfo() {};

double
MCParticleInfo::getD0()
{

  double pt = this->getPt();
  double px = this->getPx();
  double py = this->getPy();
  double x = this->getX();
  double y = this->getY();

  double qe = 1;
  double R = pt / 0.3 / m_magField.Mag() / qe * 100; //cm
  double alpha =  R / pt * m_charge; //cm/GeV
  double Cx = x + alpha * py; //cm
  double Cy = y - alpha * px; //cm
  double d0 = sqrt(Cx * Cx + Cy * Cy) - R; //cm
  d0 = d0 * m_charge;

  return d0;
};

double
MCParticleInfo::getPhi()
{

  //this is an approximation
  return  atan2(this->getPy(), this->getPx());
};

double
MCParticleInfo::getOmega()
{
  double R = this->getPt() / m_magField.Mag() / 0.3 / m_charge * 100; // cm
  return 1 / R;
};

double
MCParticleInfo::getZ0()
{
  //to be implemented
  return -999;
};

double
MCParticleInfo::getCotTheta()
{
  //to be implemented
  return -999;
};

