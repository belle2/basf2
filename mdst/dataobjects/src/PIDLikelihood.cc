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

#include <cmath>

using namespace std;
using namespace Belle2;

PIDLikelihood::PIDLikelihood()
{
  for (unsigned short i = 0; i < Const::PIDDetectors::c_size; i++) {
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
  if (logl != logl or logl == INFINITY) {
    B2ERROR("PIDLikelihood::setLogLikelihood: log-likelihood for detector " << det << " is " << logl << " (i.e. +inf or NaN)! Ignoring this value. (" << Const::CDC << "=CDC, " << Const::TOP << "=TOP, " << Const::ARICH << "=ARICH, " << Const::ECL << "=ECL)");

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


double PIDLikelihood::getProbability(const Const::ChargedStable& p1,
                                     const Const::ChargedStable& p2,
                                     double ratio,
                                     Const::PIDDetectorSet set) const
{
  if (ratio < 0) {
    B2ERROR("PIDLikelihood::probability argument 'ratio' is given with negative value");
    return 0;
  }
  if (ratio == 0) return 0;

  double dlogl = getLogL(p2, set) - getLogL(p1, set);
  double res;
  if (dlogl < 0) {
    double elogl = exp(dlogl);
    res = ratio / (ratio + elogl);
  } else {
    double elogl = exp(-dlogl) * ratio; // to prevent overflow for very large dlogl
    res = elogl / (1.0 + elogl);
  }
  //TODO: only necessary if one wants to use mcprod1405 MC sample. Remove when there's a good replacement.
  if (std::isfinite(res))
    return res;
  return 0;
}

double PIDLikelihood::getProbability(const Const::ChargedStable& part,
                                     const double* fractions,
                                     Const::PIDDetectorSet detSet) const
{
  const unsigned int n = Const::chargedStableSet.size();
  double prob[n];
  probability(prob, fractions, detSet);

  int k = part.getIndex();
  if (k < 0) return 0;

  return prob[k];

}

Const::ChargedStable PIDLikelihood::getMostLikely(const double* fractions,
                                                  Const::PIDDetectorSet detSet) const
{
  const unsigned int n = Const::chargedStableSet.size();
  double prob[n];
  probability(prob, fractions, detSet);

  int k = 0;
  double maxProb = prob[k];
  for (unsigned i = 0; i < n; ++i) {
    if (prob[i] > maxProb) {maxProb = prob[i]; k = i;}
  }
  return Const::chargedStableSet.at(k);

}


void PIDLikelihood::probability(double probabilities[],
                                const double* fractions,
                                Const::PIDDetectorSet detSet) const
{
  const unsigned int n = Const::chargedStableSet.size();
  double frac[n];
  if (!fractions) {
    for (unsigned int i = 0; i < n; ++i) frac[i] = 1.0; // normalization not needed
    fractions = frac;
  }

  double logL[n];
  double logLmax = 0;
  bool hasMax = false;
  for (unsigned i = 0; i < n; ++i) {
    logL[i] = 0;
    if (fractions[i] > 0) {
      logL[i] = getLogL(Const::chargedStableSet.at(i), detSet);
      if (!hasMax || logL[i] > logLmax) {
        logLmax = logL[i];
        hasMax = true;
      }
    }
  }

  double norm = 0;
  for (unsigned i = 0; i < n; ++i) {
    probabilities[i] = 0;
    if (fractions[i] > 0) probabilities[i] = exp(logL[i] - logLmax) * fractions[i];
    norm += probabilities[i];
  }
  if (norm == 0) return;

  for (unsigned i = 0; i < n; ++i) {
    probabilities[i] /= norm;
  }

}


ClassImp(PIDLikelihood)


