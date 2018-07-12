/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/dbobjects/ARICHSimulationPar.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Belle2;

double ARICHSimulationPar::getQE(double e) const
{
  if (e < 0.001) return 0;
  if (m_qe.size() == 0) B2ERROR("ARICHSimulationPar: QE curve not initialized!");
  double dlam = 1240 / e - m_lambdaFirst;
  if (dlam < 0) return 0;
  int i = int(dlam / m_lambdaStep);
  if (i > int(m_qe.size()) - 2) return 0;

  // linear interpolation
  double qe = m_qe[i] + (m_qe[i + 1] - m_qe[i]) / m_lambdaStep * (dlam - i * m_lambdaStep);
  return qe * m_peakQE;
}

void ARICHSimulationPar::setQECurve(float lambdaFirst, float lambdaStep, const std::vector<float>& qe)
{
  m_lambdaFirst = lambdaFirst;
  m_lambdaStep = lambdaStep;
  m_qe = qe;
}

void ARICHSimulationPar::print() const
{
  cout << endl;
  cout << "ARICH simulation parameters" << endl;
  cout << endl;
  cout << " QE curve (peak should be above QE of highest QE HAPD!)" << endl;
  int ipoint = 0;
  cout << "  wl [nm]   QE" << endl;
  for (auto qe : m_qe) {
    float lambda = m_lambdaFirst + ipoint * m_lambdaStep;
    cout << "  " << lambda << "      " << qe* m_peakQE << endl;
    ipoint++;
  }

  cout << endl;
  cout << " HAPD simulation parameters" << endl;
  cout << "  QE scaling for photons internally reflected in HAPD window: " << getQEScaling() << endl;
  cout << "  absorbtion probablity for internally reflected photons:     " << getWindowAbsorbtion() << endl;
  cout << "  channels negative crosstalk factor:                         " << getChipNegativeCrosstalk() << endl;

}
