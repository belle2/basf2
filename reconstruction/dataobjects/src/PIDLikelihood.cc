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

#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>

using namespace std;
using namespace Belle2;

void PIDLikelihood::setLikelihoods(const TOPLikelihood* logl)
{

  if (logl->getFlag() != 1) return;

  setFlag(c_Top);
  m_logl[c_Top][Const::electron.getIndex()] = (float) logl->getLogL_e();
  m_logl[c_Top][Const::muon.getIndex()] = (float) logl->getLogL_mu();
  m_logl[c_Top][Const::pion.getIndex()] = (float) logl->getLogL_pi();
  m_logl[c_Top][Const::kaon.getIndex()] = (float) logl->getLogL_K();
  m_logl[c_Top][Const::proton.getIndex()] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const ARICHLikelihoods* logl)
{

  if (logl->getFlag() != 1) return;

  setFlag(c_Arich);
  m_logl[c_Arich][Const::electron.getIndex()] = (float) logl->getLogL_e();
  m_logl[c_Arich][Const::muon.getIndex()] = (float) logl->getLogL_mu();
  m_logl[c_Arich][Const::pion.getIndex()] = (float) logl->getLogL_pi();
  m_logl[c_Arich][Const::kaon.getIndex()] = (float) logl->getLogL_K();
  m_logl[c_Arich][Const::proton.getIndex()] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const DedxLikelihood* logl)
{

  setFlag(c_Dedx);
  for (Const::ParticleType k = Const::chargedStableSet.begin(); k != Const::chargedStableSet.end(); ++k)
    m_logl[c_Dedx][k.getIndex()] = logl->getLogLikelihood(k);

}


double PIDLikelihood::getProbability(const Const::ChargedStable& p1, const Const::ChargedStable& p2) const
{

  float logl1 = 0.0;
  float logl2 = 0.0;
  for (int det = 0; det < c_NumofDet; det++) {
    logl1 += m_logl[det][p1.getIndex()];
    logl2 += m_logl[det][p2.getIndex()];
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


