/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dataobjects/Cluster.h>
#include <framework/logging/Logger.h>

#include <cmath>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

Cluster::Cluster()
{
  for (unsigned short i = 0; i < Const::ClusterDetectors::c_size; i++) {
    for (unsigned int k = 0; k < Const::Cluster::c_SetSize; k++) {
      m_logl[i][k] = 0.0;
    }
  }
}


void Cluster::setLogLikelihood(Const::EDetector det,
                               const Const::Cluster& part,
                               float logl)
{
  int index = Const::ClusterDetectors::set().getIndex(det);
  if (index < 0) {
    B2ERROR("ClusterLikelihood::setLogLikelihood: detector is not a cluster device");
    return;
  }
  if (logl != logl or logl == INFINITY) {
    B2ERROR("ClusterLikelihood::setLogLikelihood: log-likelihood for detector " << det << " is " << logl <<
            " (i.e. +inf or NaN)! Ignoring this value. (" << Const::ECL << "=ECL, " << Const::KLM << "=KLM");

    return;
  }
  m_detectors += det;
  m_logl[index][part.getIndex()] = logl;
}


float Cluster::getLogL(const Const::Cluster& cluster,
                       Const::ClusterDetectorSet set) const
{
  float result = 0;
  for (unsigned int index = 0; index < Const::ClusterDetectorSet::set().size(); ++index) {
    if (set.contains(Const::ClusterDetectorSet::set()[index]))
      result += m_logl[index][cluster.getIndex()];
  }
  return result;
}


Double32_t Cluster::getProbability(const Const::Cluster& c1,
                                   const Const::Cluster& c2,
                                   double ratio,
                                   Const::ClusterDetectorSet set) const
{
  if (ratio < 0) {
    B2ERROR("Cluster::probability argument 'ratio' is given with negative value");
    return 0;
  }
  if (ratio == 0) return 0;

  double dlogl = getLogL(c2, set) - getLogL(c1, set);
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
  return 0.0;
}

double Cluster::getProbability(const Const::Cluster& cluster,
                               const double* fractions,
                               Const::ClusterDetectorSet detSet) const
{
  double prob[Const::Cluster::c_SetSize];
  probability(prob, fractions, detSet);

  int k = cluster.getIndex();
  if (k < 0) return 0;

  return prob[k];

}

Const::Cluster Cluster::getMostLikely(const double* fractions,
                                      Const::ClusterDetectorSet detSet) const
{
  const unsigned int n = Const::Cluster::c_SetSize;
  double prob[n];
  probability(prob, fractions, detSet);

  int k = 0;
  double maxProb = prob[k];
  for (unsigned i = 0; i < n; ++i) {
    if (prob[i] > maxProb) {maxProb = prob[i]; k = i;}
  }
  return Const::clusterSet.at(k);

}


void Cluster::probability(double probabilities[],
                          const double* fractions,
                          Const::ClusterDetectorSet detSet) const
{
  const unsigned int n = Const::Cluster::c_SetSize;
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
      logL[i] = getLogL(Const::clusterSet.at(i), detSet);
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


void Cluster::printArray() const
{

  string detectorName[Const::ClusterDetectors::c_size] = {"ECL", "KLM"};
  string hline("-------");
  for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++)
    hline += "-----------";
  string Hline("=======");
  for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++)
    Hline += "===========";

  cout << Hline << endl;

  cout << "PDGcode";
  for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++)
    cout << setw(10) << Const::clusterSet.at(i).getPDGCode() << " ";
  cout << endl;

  cout << Hline << endl;

  float sum_logl[Const::Cluster::c_SetSize];
  for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++) sum_logl[i] = 0;

  for (unsigned k = 0; k < Const::ClusterDetectors::c_size; k++) {
    cout << setw(7) << detectorName[k];
    for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++) {
      cout << setw(10) << setprecision(4) << m_logl[k][i] << " ";
      sum_logl[i] += m_logl[k][i];
    }
    cout << endl;
  }

  cout << hline << endl;

  cout << setw(7) << "sum";
  for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++)
    cout << setw(10) << setprecision(4) << sum_logl[i] << " ";
  cout << endl;

  if (isAvailable(Const::ECL) or isAvailable(Const::KLM)) {
    unsigned k = 0;
    for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++)
      if (sum_logl[i] > sum_logl[k]) k = i;
    unsigned pos = 11 * (k + 1) + 3;
    Hline.replace(pos, 1, "^");
  }
  cout << Hline << endl;

}

std::string Cluster::getInfoHTML() const
{
  string detectorName[Const::ClusterDetectors::c_size] = {"ECL", "KLM"};

  std::stringstream stream;
  stream << std::setprecision(4);
  stream << "<b>Likelihoods</b><br>";
  stream << "<table>";
  stream << "<tr><th>Cluster / Detector</th>";
  for (unsigned k = 0; k < Const::ClusterDetectors::c_size; k++)
    stream << "<th>" << detectorName[k] << "</th>";
  stream << "</tr>";
  for (unsigned i = 0; i < Const::Cluster::c_SetSize; i++) {
    stream << "<tr>";
    stream << "<td>" << Const::clusterSet.at(i).getPDGCode() << "</td>";
    for (unsigned k = 0; k < Const::ClusterDetectors::c_size; k++) {
      stream << "<td>" << m_logl[k][i] << "</td>";
    }
    stream << "</tr>";
  }
  stream << "</table>";
  stream << "<br>";

  return stream.str();
}


