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

PIDLikelihood::PIDLikelihood()
{
  for (unsigned short i = 0; i < Const::PIDDetectors::set().size(); i++) {
    for (unsigned int k = 0; k < Const::chargedStableSet.size(); ++k) {
      m_logl[i][k] = 0.0;
    }
  }
}

void PIDLikelihood::setLikelihoods(const TOPLikelihood* logl)
{

  if (logl->getFlag() != 1) return;

  m_detectors += Const::TOP;
  int index = Const::PIDDetectors::set().getIndex(Const::TOP);
  m_logl[index][Const::electron.getIndex()] = (float) logl->getLogL_e();
  m_logl[index][Const::muon.getIndex()] = (float) logl->getLogL_mu();
  m_logl[index][Const::pion.getIndex()] = (float) logl->getLogL_pi();
  m_logl[index][Const::kaon.getIndex()] = (float) logl->getLogL_K();
  m_logl[index][Const::proton.getIndex()] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const ARICHLikelihoods* logl)
{

  if (logl->getFlag() != 1) return;

  m_detectors += Const::ARICH;
  int index = Const::PIDDetectors::set().getIndex(Const::ARICH);
  m_logl[index][Const::electron.getIndex()] = (float) logl->getLogL_e();
  m_logl[index][Const::muon.getIndex()] = (float) logl->getLogL_mu();
  m_logl[index][Const::pion.getIndex()] = (float) logl->getLogL_pi();
  m_logl[index][Const::kaon.getIndex()] = (float) logl->getLogL_K();
  m_logl[index][Const::proton.getIndex()] = (float) logl->getLogL_p();
}


void PIDLikelihood::setLikelihoods(const DedxLikelihood* logl)
{

  m_detectors += Const::CDC;
  int index = Const::PIDDetectors::set().getIndex(Const::CDC);
  for (Const::ParticleType k = Const::chargedStableSet.begin(); k != Const::chargedStableSet.end(); ++k)
    m_logl[index][k.getIndex()] = logl->getLogLikelihood(k);

}


float PIDLikelihood::getLogL(const Const::ChargedStable& part, Const::PIDDetectorSet set) const
{
  float result = 0;
  for (unsigned int index = 0; index < Const::PIDDetectorSet::set().size(); ++index) {
    if (set.contains(Const::PIDDetectorSet::set()[index])) result += m_logl[index][part.getIndex()];
  }
  return result;
}

double PIDLikelihood::getProbability(const Const::ChargedStable& p1, const Const::ChargedStable& p2, Const::PIDDetectorSet set) const
{
  return probability(getLogL(p1, set), getLogL(p2, set));
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


