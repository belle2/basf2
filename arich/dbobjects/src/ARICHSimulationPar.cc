/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/dbobjects/ARICHSimulationPar.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace std;
using namespace Belle2;

double ARICHSimulationPar::getQE(double e) const
{
  if (e < 0.001) return 0;
  if (m_qe.size() == 0) B2ERROR("ARICHSimulationPar: QE curve not initialized!");
  double dlam = 1240 / e - m_lambdaFirst;

  int i = int(dlam / m_lambdaStep);
  if (i < 0) i = 0;
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
  cout << "  collection efficiency:                                      " << m_colEff << endl;
}
