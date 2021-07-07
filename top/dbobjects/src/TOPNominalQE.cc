/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPNominalQE.h>
#include <iostream>

using namespace std;

namespace Belle2 {


  void TOPNominalQE::applyFilterTransmission(const TOPWavelengthFilter& filter)
  {
    for (size_t i = 0; i < m_QE.size(); i++) {
      double lambda = m_lambdaFirst + m_lambdaStep * i;
      m_QE[i] *= filter.getBulkTransmittance(lambda);
    }
  }


  double TOPNominalQE::getQE(double lambda) const
  {
    double dlam = lambda - m_lambdaFirst;
    if (dlam < 0 or dlam > (m_QE.size() - 1) * m_lambdaStep) return 0;
    unsigned i = int(dlam / m_lambdaStep);
    if (i > m_QE.size() - 2) return m_QE.back();
    return m_QE[i] + (m_QE[i + 1] - m_QE[i]) / m_lambdaStep * (dlam - i * m_lambdaStep);
  }


  double TOPNominalQE::getMinLambda() const
  {
    for (size_t i = 0; i < m_QE.size(); i++) {
      if (m_QE[i] > 0) return m_lambdaFirst + m_lambdaStep * i;
    }
    return getLambdaLast();
  }


  double TOPNominalQE::getMaxLambda() const
  {
    for (size_t i = m_QE.size() - 1; i < m_QE.size(); i--) {
      if (m_QE[i] > 0) return m_lambdaFirst + m_lambdaStep * i;
    }
    return m_lambdaFirst;
  }


  bool TOPNominalQE::isConsistent() const
  {
    if (m_lambdaFirst <= 0) return false;
    if (m_lambdaStep <= 0) return false;
    if (m_CE <= 0) return false;
    if (m_QE.empty()) return false;
    return true;
  }


  void TOPNominalQE::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);
    cout << " first point: " << getLambdaFirst() << " nm";
    cout << ", step: " << getLambdaStep() << " nm";
    cout << ", CE: " << getCE() << endl;
    cout << " QE: [";
    for (const auto& qe : m_QE) cout << qe << ", ";
    cout << "]" << endl;
  }

} // end Belle2 namespace
