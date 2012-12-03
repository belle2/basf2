/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/dataobjects/PIDLikelihood.h>

using namespace std;
using namespace Belle2;

void PIDLikelihood::setLikelihoods(const TOPLikelihood* logl)
{

  if (logl->getFlag() != 1) return;

  setFlag(c_Top);
  m_logl[c_Top][PIDLikelihood::c_Electron] = (float) logl->getLogL_e();
  m_logl[c_Top][PIDLikelihood::c_Muon] = (float) logl->getLogL_mu();
  m_logl[c_Top][PIDLikelihood::c_Pion] = (float) logl->getLogL_pi();
  m_logl[c_Top][PIDLikelihood::c_Kaon] = (float) logl->getLogL_K();
  m_logl[c_Top][PIDLikelihood::c_Proton] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const ARICHLikelihoods* logl)
{

  if (logl->getFlag() != 1) return;

  setFlag(c_Arich);
  m_logl[c_Arich][PIDLikelihood::c_Electron] = (float) logl->getLogL_e();
  m_logl[c_Arich][PIDLikelihood::c_Muon] = (float) logl->getLogL_mu();
  m_logl[c_Arich][PIDLikelihood::c_Pion] = (float) logl->getLogL_pi();
  m_logl[c_Arich][PIDLikelihood::c_Kaon] = (float) logl->getLogL_K();
  m_logl[c_Arich][PIDLikelihood::c_Proton] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const DedxLikelihood* logl)
{

  setFlag(c_Dedx);
  m_logl[c_Dedx][PIDLikelihood::c_Electron] = logl->getLogLikelihood(Dedx::c_Electron);
  m_logl[c_Dedx][PIDLikelihood::c_Muon] = logl->getLogLikelihood(Dedx::c_Muon);
  m_logl[c_Dedx][PIDLikelihood::c_Pion] = logl->getLogLikelihood(Dedx::c_Pion);
  m_logl[c_Dedx][PIDLikelihood::c_Kaon] = logl->getLogLikelihood(Dedx::c_Kaon);
  m_logl[c_Dedx][PIDLikelihood::c_Proton] = logl->getLogLikelihood(Dedx::c_Proton);
}


double PIDLikelihood::getProbability(EParticle p1, EParticle p2) const
{

  float logl1 = 0.0;
  float logl2 = 0.0;
  for (int det = 0; det < c_NumofDet; det++) {
    logl1 += m_logl[det][p1];
    logl2 += m_logl[det][p2];
  }
  return probability(logl1, logl2);
}


double PIDLikelihood::probability(float logl1, float logl2) const
{
  double dlogl = logl2 - logl1;
  if (dlogl < 0) {
    double elogl = exp(dlogl);
    return 1.0 / (1.0 + elogl);
  } else {
    double elogl = exp(-dlogl); // to prevent overflow for very large dlogl
    return elogl / (1.0 + elogl);
  }
}


ClassImp(PIDLikelihood)


