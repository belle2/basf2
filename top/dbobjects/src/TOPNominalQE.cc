/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPNominalQE.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  double TOPNominalQE::getQE(double energy) const
  {
    double dlam = 1240 / energy - m_lambdaFirst;
    if (dlam < 0 or dlam >= (m_QE.size() - 1) * m_lambdaStep) return 0;
    unsigned i = int(dlam / m_lambdaStep);
    if (i > m_QE.size() - 2) return 0;
    return m_QE[i] + (m_QE[i + 1] - m_QE[i]) / m_lambdaStep * (dlam - i * m_lambdaStep);
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
