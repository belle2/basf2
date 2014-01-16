/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <mdst/dataobjects/PIDLikelihood.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

PIDLikelihood::PIDLikelihood()
{
  //  for (unsigned short i = 0; i < Const::PIDDetectors::set().size(); i++) {
  //    for (unsigned int k = 0; k < Const::chargedStableSet.size(); ++k) {

  if (Const::PIDDetectors::set().size() > c_PIDDetectorSetSize)
    B2FATAL("PIDLikelihood::m_logl[][] first dimension too small");

  for (unsigned short i = 0; i < c_PIDDetectorSetSize; i++) {
    for (unsigned int k = 0; k < Const::ChargedStable::c_SetSize; k++) {
      m_logl[i][k] = 0.0;
    }
  }
}


void PIDLikelihood::setLogLikelihood(Const::EDetector det,
                                     const Const::ChargedStable& part,
                                     float logl)
{
  int index = Const::PIDDetectors::set().getIndex(det);
  if (index < 0) {
    B2ERROR("PIDLikelihood::setLogLikelihood: detector is not a PID device");
    return;
  }
  m_detectors += det;
  m_logl[index][part.getIndex()] = logl;
}


float PIDLikelihood::getLogL(const Const::ChargedStable& part,
                             Const::PIDDetectorSet set) const
{
  float result = 0;
  for (unsigned int index = 0; index < Const::PIDDetectorSet::set().size(); ++index) {
    if (set.contains(Const::PIDDetectorSet::set()[index]))
      result += m_logl[index][part.getIndex()];
  }
  return result;
}


double PIDLikelihood::getProbability(const Const::ChargedStable& part,
                                     const float* fractions,
                                     Const::PIDDetectorSet detSet) const
{
  unsigned n = Const::chargedStableSet.size();
  double frac[n];
  if (fractions) {
    frac[Const::electron.getIndex()] = fractions[0];
    frac[Const::muon.getIndex()]     = fractions[1];
    frac[Const::pion.getIndex()]     = fractions[2];
    frac[Const::kaon.getIndex()]     = fractions[3];
    frac[Const::proton.getIndex()]   = fractions[4];
  } else {
    for (unsigned i = 0; i < n; ++i) frac[i] = 1.0; // normalization not needed
  }
  double prob[n];
  probability(n, frac, prob, detSet);

  int k = part.getIndex();
  if (k < 0) return 0;

  return prob[k];

}

Const::ChargedStable PIDLikelihood::getMostLikely(const float* fractions,
                                                  Const::PIDDetectorSet detSet) const
{
  unsigned n = Const::chargedStableSet.size();
  double frac[n];
  if (fractions) {
    frac[Const::electron.getIndex()] = fractions[0];
    frac[Const::muon.getIndex()]     = fractions[1];
    frac[Const::pion.getIndex()]     = fractions[2];
    frac[Const::kaon.getIndex()]     = fractions[3];
    frac[Const::proton.getIndex()]   = fractions[4];
  } else {
    for (unsigned i = 0; i < n; ++i) frac[i] = 1.0; // normalization not needed
  }
  double prob[n];
  probability(n, frac, prob, detSet);

  int k = 0;
  double maxProb = prob[k];
  for (unsigned i = 0; i < n; ++i) {
    if (prob[i] > maxProb) {maxProb = prob[i]; k = i;}
  }
  return Const::chargedStableSet.at(k);

}


double PIDLikelihood::probability(float logl1, float logl2, double ratio) const
{
  if (ratio < 0) {
    B2ERROR("PIDLikelihood::probability argument 'ratio' is given with negative value");
    return 0;
  }
  if (ratio == 0) return 0;

  double dlogl = logl2 - logl1;
  if (dlogl < 0) {
    double elogl = exp(dlogl);
    return ratio / (ratio + elogl);
  } else {
    double elogl = exp(-dlogl) * ratio; // to prevent overflow for very large dlogl
    return elogl / (1.0 + elogl);
  }
}


void PIDLikelihood::probability(unsigned n,
                                double frac[],
                                double prob[],
                                Const::PIDDetectorSet detSet) const
{
  double logL[n];
  unsigned i0 = 0;
  for (unsigned i = 0; i < n; ++i) {
    logL[i] = 0;
    if (frac[i] > 0) {
      logL[i] = getLogL(Const::chargedStableSet.at(i), detSet);
      i0 = i;
    }
  }

  double logLmax = logL[i0];
  for (unsigned i = 0; i < n; ++i) {
    if (frac[i] > 0 && logL[i] > logLmax) logLmax = logL[i];
  }

  double norm = 0;
  for (unsigned i = 0; i < n; ++i) {
    prob[i] = 0;
    if (frac[i] > 0) prob[i] = exp(logL[i] - logLmax) * frac[i];
    norm += prob[i];
  }
  if (norm == 0) return;

  for (unsigned i = 0; i < n; ++i) {
    prob[i] /= norm;
  }

}


ClassImp(PIDLikelihood)


