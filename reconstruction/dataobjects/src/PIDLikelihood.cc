/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Thomas Kuhr                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/dataobjects/PIDLikelihood.h>
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


