/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * RecoilMassConstraint for OrcaKinFit. Allows constraints of the type    *
 * (P_beam - P) - RecoilMass = 0                                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "analysis/OrcaKinFit/RecoilMassConstraint.h"
#include "analysis/OrcaKinFit/ParticleFitObject.h"

#include<iostream>
#include<cmath>

#undef NDEBUG
#include<cassert>

// constructor
RecoilMassConstraint::RecoilMassConstraint(double recoilmass, double beampx, double beampy, double beampz, double beampe)
  : m_recoilMass(recoilmass), m_beamPx(beampx), m_beamPy(beampy), m_beamPz(beampz), m_beamE(beampe)
{}

// destructor
RecoilMassConstraint::~RecoilMassConstraint()
{
  ;
}

// calulate current value of constraint function
double RecoilMassConstraint::getValue() const
{
  double totE = 0.;
  double totpx = 0.;
  double totpy = 0.;
  double totpz = 0.;

  for (unsigned int i = 0; i < fitobjects.size(); i++) {
    ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
    assert(pfo);
    totE  += pfo->getE();
    totpx += pfo->getPx();
    totpy += pfo->getPy();
    totpz += pfo->getPz();
  }

  const double recoilE   = (m_beamE - totE);
  const double recoilE2  = recoilE * recoilE;
  const double recoilpx  = (m_beamPx - totpx);
  const double recoilpx2 = recoilpx * recoilpx;
  const double recoilpy  = (m_beamPy - totpy);
  const double recoilpy2 = recoilpy * recoilpy;
  const double recoilpz  = (m_beamPz - totpz);
  const double recoilpz2 = recoilpz * recoilpz;
  const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
  const double recoil    = std::sqrt(std::fabs(recoil2));
  const double result    = recoil - m_recoilMass;
  return result;
}

// calculate vector/array of derivatives of this contraint
// w.r.t. to ALL parameters of all fitobjects
// here: d RM /d par(j)
//          = d RM /d p(i) * d p(i) /d par(j)
//          =  +-1/RM * (p(i)-c(i)) * d p(i) /d par(j)

void RecoilMassConstraint::getDerivatives(int idim, double der[]) const
{

  double totE = 0.;
  double totpx = 0.;
  double totpy = 0.;
  double totpz = 0.;

  for (unsigned int i = 0; i < fitobjects.size(); i++) {
    ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
    assert(pfo);
    totE  += pfo->getE();
    totpx += pfo->getPx();
    totpy += pfo->getPy();
    totpz += pfo->getPz();
  }

  const double recoilE   = (m_beamE - totE);
  const double recoilE2  = recoilE * recoilE;
  const double recoilpx  = (m_beamPx - totpx);
  const double recoilpx2 = recoilpx * recoilpx;
  const double recoilpy  = (m_beamPy - totpy);
  const double recoilpy2 = recoilpy * recoilpy;
  const double recoilpz  = (m_beamPz - totpz);
  const double recoilpz2 = recoilpz * recoilpz;
  const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
  const double recoil    = std::sqrt(std::fabs(recoil2));

  double recoilmass_inv = 0.;
  if (recoil > 1e-9)  recoilmass_inv = 1. / recoil;

  for (unsigned int i = 0; i < fitobjects.size(); i++) {
    for (int ilocal = 0; ilocal < fitobjects[i]->getNPar(); ilocal++) {
      if (!fitobjects[i]->isParamFixed(ilocal)) {
        int iglobal = fitobjects[i]->getGlobalParNum(ilocal);
        assert(iglobal >= 0 && iglobal < idim);

        ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
        assert(pfo);
        der[iglobal] = - recoilE  * pfo->getDE(ilocal)
                       + recoilpx * pfo->getDPx(ilocal)
                       + recoilpy * pfo->getDPy(ilocal)
                       + recoilpz * pfo->getDPz(ilocal);
        der[iglobal] *= recoilmass_inv;
      }
    }
  }
}


double RecoilMassConstraint::getRecoilMass()
{
  double totE = 0.;
  double totpx = 0.;
  double totpy = 0.;
  double totpz = 0.;

  for (unsigned int i = 0; i < fitobjects.size(); i++) {
    ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
    assert(pfo);
    totE  += pfo->getE();
    totpx += pfo->getPx();
    totpy += pfo->getPy();
    totpz += pfo->getPz();
  }

  const double recoilE   = (m_beamE - totE);
  const double recoilE2  = recoilE * recoilE;
  const double recoilpx  = (m_beamPx - totpx);
  const double recoilpx2 = recoilpx * recoilpx;
  const double recoilpy  = (m_beamPy - totpy);
  const double recoilpy2 = recoilpy * recoilpy;
  const double recoilpz  = (m_beamPz - totpz);
  const double recoilpz2 = recoilpz * recoilpz;
  const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
  const double recoil    = std::sqrt(std::fabs(recoil2));

  return recoil;
}

void RecoilMassConstraint::setRecoilMass(double recoilmass_)
{
  m_recoilMass = recoilmass_;
}

bool RecoilMassConstraint::secondDerivatives(int i, int j, double* dderivatives) const
{
  (void) i;
  (void) j;
  double totE = 0.;
  double totpx = 0.;
  double totpy = 0.;
  double totpz = 0.;

  for (unsigned int i = 0; i < fitobjects.size(); i++) {
    ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
    assert(pfo);
    totE  += pfo->getE();
    totpx += pfo->getPx();
    totpy += pfo->getPy();
    totpz += pfo->getPz();
  }

  const double recoilE   = (m_beamE - totE);
  const double recoilE2  = recoilE * recoilE;
  const double recoilpx  = (m_beamPx - totpx);
  const double recoilpx2 = recoilpx * recoilpx;
  const double recoilpy  = (m_beamPy - totpy);
  const double recoilpy2 = recoilpy * recoilpy;
  const double recoilpz  = (m_beamPz - totpz);
  const double recoilpz2 = recoilpz * recoilpz;
  const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
  const double recoil    = sqrt(recoil2);

  assert(dderivatives);
  for (int k = 0; k < 16; ++k) dderivatives[k] = 0;

  if (recoil > 1e-12)  {
    double recoilmass_inv3 = 1. / (recoil * recoil * recoil);

    dderivatives[4 * 0 + 0] = (recoil2 - recoilE2) * recoilmass_inv3;                  //dE^2
    dderivatives[4 * 0 + 1] = dderivatives[4 * 1 + 0] =  recoilE * recoilpx * recoilmass_inv3; //dEdpx
    dderivatives[4 * 0 + 2] = dderivatives[4 * 2 + 0] =  recoilE * recoilpy * recoilmass_inv3; //dEdpy
    dderivatives[4 * 0 + 3] = dderivatives[4 * 3 + 0] =  recoilE * recoilpz * recoilmass_inv3; //dEdpz
    dderivatives[4 * 1 + 1] =                       -(recoil2 + recoilpx2) * recoilmass_inv3; //dpx^2
    dderivatives[4 * 1 + 2] = dderivatives[4 * 2 + 1] = -recoilpx * recoilpy * recoilmass_inv3; //dpxdpy
    dderivatives[4 * 1 + 3] = dderivatives[4 * 3 + 1] = -recoilpx * recoilpz * recoilmass_inv3; //dpxdpz
    dderivatives[4 * 2 + 2] =                       -(recoil2 + recoilpy2) * recoilmass_inv3; //dpy^2
    dderivatives[4 * 2 + 3] = dderivatives[4 * 3 + 2] = -recoilpy * recoilpz * recoilmass_inv3; //dpydpz
    dderivatives[4 * 3 + 3] =                       -(recoil2 + recoilpz2) * recoilmass_inv3; //dpz^2
  }

  return true;

}

bool RecoilMassConstraint::firstDerivatives(int i, double* dderivatives) const
{
  (void) i;

  double totE = 0.;
  double totpx = 0.;
  double totpy = 0.;
  double totpz = 0.;

  for (unsigned int i = 0; i < fitobjects.size(); i++) {
    ParticleFitObject* pfo = dynamic_cast < ParticleFitObject* >(fitobjects[i]);
    assert(pfo);
    totE  += pfo->getE();
    totpx += pfo->getPx();
    totpy += pfo->getPy();
    totpz += pfo->getPz();
  }

  const double recoilE   = (m_beamE - totE);
  const double recoilE2  = recoilE * recoilE;
  const double recoilpx  = (m_beamPx - totpx);
  const double recoilpx2 = recoilpx * recoilpx;
  const double recoilpy  = (m_beamPy - totpy);
  const double recoilpy2 = recoilpy * recoilpy;
  const double recoilpz  = (m_beamPz - totpz);
  const double recoilpz2 = recoilpz * recoilpz;
  const double recoil2   = recoilE2 - recoilpx2 - recoilpy2 - recoilpz2;
  const double recoil    = sqrt(recoil2);


  dderivatives[0] = -recoilE / recoil;
  dderivatives[1] = recoilpx / recoil;
  dderivatives[2] = recoilpy / recoil;
  dderivatives[3] = recoilpz / recoil;

  return true;
}

int RecoilMassConstraint::getVarBasis() const
{
  return VAR_BASIS;
}
