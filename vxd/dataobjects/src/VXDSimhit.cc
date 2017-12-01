/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2014  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Peter Kvasnicka, Martin Ritter             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/dataobjects/VXDElectronDeposit.h>
#include <vxd/dataobjects/VXDSimHit.h>

using namespace std;
using namespace Belle2;

float VXDSimHit::getElectrons() const
{
  VXDElectronDeposit total(m_electronProfile.back());
  return total.getElectrons();
}

std::vector<std::pair<float, float>> VXDSimHit::getElectronProfile() const
{
  std::vector<std::pair<float, float>> result;
  result.reserve(m_electronProfile.size());
  for (unsigned int encoded : m_electronProfile) {
    VXDElectronDeposit ed(encoded);
    result.emplace_back(ed.getFraction(), ed.getElectrons());
  }
  return result;
}

std::vector<std::pair<float, float>> VXDSimHit::getElectronsConstantDistance(double length) const
{
  double totalLength = (getPosOut() - getPosIn()).Mag();
  const int nSteps = (int)(totalLength / length) + 1;
  std::vector<std::pair<float, float>> result;
  result.reserve(nSteps);
  //Account for the discrete number of steps and adjust the fraction per step
  //we want to return
  const double deltaFraction = 1. / nSteps;

  std::vector<unsigned int>::const_iterator currentPointIt = m_electronProfile.begin();
  VXDElectronDeposit currentPoint(*currentPointIt);
  VXDElectronDeposit lastPoint(0, 0);

  //Now we create all steps
  for (int i = 0; i < nSteps; ++i) {
    //By determining which fraction it should have, clipped to 1 for rounding errors
    const double fraction = min((i + 1) * deltaFraction, 1.0);
    //finding the correct segment
    while (fraction > currentPoint.getFraction()) {
      ++currentPointIt;
      lastPoint = currentPoint;
      currentPoint = VXDElectronDeposit(*currentPointIt);
    }
    //and calculating the weighted average of the current number of electrons
    const double weight = (fraction - lastPoint.getFraction()) /
                          (currentPoint.getFraction() - lastPoint.getFraction());
    const double electrons = (1 - weight) * lastPoint.getElectrons() + weight * currentPoint.getElectrons();
    //which we then add to the result
    result.emplace_back(fraction, electrons);
  }
  return result;
}

std::vector<std::pair<float, float>> VXDSimHit::getElectronsConstantNumber(double electronsPerStep) const
{
  VXDElectronDeposit total(m_electronProfile.back());
  const int nSteps = (int)(total.getElectrons() / electronsPerStep) + 1;
  std::vector<std::pair<float, float>> result;
  result.reserve(nSteps);
  //Account for the discrete number of steps and adjust number of electrons
  double deltaElectrons = total.getElectrons() / nSteps;

  std::vector<unsigned int>::const_iterator currentPointIt = m_electronProfile.begin();
  VXDElectronDeposit currentPoint(*currentPointIt);
  VXDElectronDeposit lastPoint(0, 0);

  //Now we create all steps
  for (int i = 0; i < nSteps; ++i) {
    //By determining the number of electrons it should have, clipped to total for rounding errors
    const double electrons = min((i + 1) * deltaElectrons, (double)total.getElectrons());
    //finding the correct segment
    while (electrons > currentPoint.getElectrons()) {
      ++currentPointIt;
      lastPoint = currentPoint;
      currentPoint = VXDElectronDeposit(*currentPointIt);
    }
    //and calculating the weighted average of the current position
    const double weight = (electrons - lastPoint.getElectrons()) /
                          (currentPoint.getElectrons() - lastPoint.getElectrons());
    const double fraction = (1 - weight) * lastPoint.getFraction() + weight * currentPoint.getFraction();
    //which we then add to the result
    result.emplace_back(fraction, electrons);
  }
  return result;
}
