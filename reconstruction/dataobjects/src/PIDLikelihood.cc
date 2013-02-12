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
  m_logl[c_Top][Const::ChargedStable::electron.getIndex()] = (float) logl->getLogL_e();
  m_logl[c_Top][Const::ChargedStable::muon.getIndex()] = (float) logl->getLogL_mu();
  m_logl[c_Top][Const::ChargedStable::pion.getIndex()] = (float) logl->getLogL_pi();
  m_logl[c_Top][Const::ChargedStable::kaon.getIndex()] = (float) logl->getLogL_K();
  m_logl[c_Top][Const::ChargedStable::proton.getIndex()] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const ARICHLikelihoods* logl)
{

  if (logl->getFlag() != 1) return;

  setFlag(c_Arich);
  m_logl[c_Arich][Const::ChargedStable::electron.getIndex()] = (float) logl->getLogL_e();
  m_logl[c_Arich][Const::ChargedStable::muon.getIndex()] = (float) logl->getLogL_mu();
  m_logl[c_Arich][Const::ChargedStable::pion.getIndex()] = (float) logl->getLogL_pi();
  m_logl[c_Arich][Const::ChargedStable::kaon.getIndex()] = (float) logl->getLogL_K();
  m_logl[c_Arich][Const::ChargedStable::proton.getIndex()] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const DedxLikelihood* logl)
{

  setFlag(c_Dedx);
  m_logl[c_Dedx][Const::ChargedStable::electron.getIndex()] = logl->getLogLikelihood(Dedx::c_Electron);
  m_logl[c_Dedx][Const::ChargedStable::muon.getIndex()] = logl->getLogLikelihood(Dedx::c_Muon);
  m_logl[c_Dedx][Const::ChargedStable::pion.getIndex()] = logl->getLogLikelihood(Dedx::c_Pion);
  m_logl[c_Dedx][Const::ChargedStable::kaon.getIndex()] = logl->getLogLikelihood(Dedx::c_Kaon);
  m_logl[c_Dedx][Const::ChargedStable::proton.getIndex()] = logl->getLogLikelihood(Dedx::c_Proton);
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


