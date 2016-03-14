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
#include <iostream>
#include <iomanip>

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
    B2ERROR("PIDLikelihood::setLogLikelihood: log-likelihood for detector " << det << " is " << logl <<
            " (i.e. +inf or NaN)! Ignoring this value. (" << Const::CDC << "=CDC, " << Const::TOP << "=TOP, " << Const::ARICH << "=ARICH, " <<
            Const::ECL << "=ECL)");

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
  double prob[Const::ChargedStable::c_SetSize];
  probability(prob, fractions, detSet);

  int k = part.getIndex();
  if (k < 0) return 0;

  return prob[k];

}

Const::ChargedStable PIDLikelihood::getMostLikely(const double* fractions,
                                                  Const::PIDDetectorSet detSet) const
{
  const unsigned int n = Const::ChargedStable::c_SetSize;
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
  const unsigned int n = Const::ChargedStable::c_SetSize;
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


void PIDLikelihood::printArray() const
{

  string detectorName[Const::PIDDetectors::c_size] =
  {"SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"};
  string hline("-------");
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    hline += "-----------";
  string Hline("=======");
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    Hline += "===========";

  cout << Hline << endl;

  cout << "PDGcode";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    cout << setw(10) << Const::chargedStableSet.at(i).getPDGCode() << " ";
  cout << endl;

  cout << Hline << endl;

  float sum_logl[Const::ChargedStable::c_SetSize];
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) sum_logl[i] = 0;

  for (unsigned k = 0; k < Const::PIDDetectors::c_size; k++) {
    cout << setw(7) << detectorName[k];
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
      cout << setw(10) << setprecision(4) << m_logl[k][i] << " ";
      sum_logl[i] += m_logl[k][i];
    }
    cout << endl;
  }

  cout << hline << endl;

  cout << setw(7) << "sum";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    cout << setw(10) << setprecision(4) << sum_logl[i] << " ";
  cout << endl;

  if (isAvailable(Const::SVD) or isAvailable(Const::CDC) or isAvailable(Const::TOP) or
      isAvailable(Const::ARICH) or isAvailable(Const::ECL) or isAvailable(Const::KLM)) {
    unsigned k = 0;
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
      if (sum_logl[i] > sum_logl[k]) k = i;
    unsigned pos = 11 * (k + 1) + 3;
    Hline.replace(pos, 1, "^");
  }
  cout << Hline << endl;

}

std::string PIDLikelihood::getInfoHTML() const
{
  string detectorName[Const::PIDDetectors::c_size] = {"SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"};

  std::stringstream stream;
  stream << std::setprecision(4);
  stream << "<b>Likelihoods</b><br>";
  stream << "<table>";
  stream << "<tr><th>PID / Detector</th>";
  for (unsigned k = 0; k < Const::PIDDetectors::c_size; k++)
    stream << "<th>" << detectorName[k] << "</th>";
  stream << "</tr>";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
    stream << "<tr>";
    stream << "<td>" << Const::chargedStableSet.at(i).getPDGCode() << "</td>";
    for (unsigned k = 0; k < Const::PIDDetectors::c_size; k++) {
      stream << "<td>" << m_logl[k][i] << "</td>";
    }
    stream << "</tr>";
  }
  stream << "</table>";
  stream << "<br>";

  return stream.str();
}


