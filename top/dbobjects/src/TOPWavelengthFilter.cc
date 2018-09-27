/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rikuya Okuto                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPWavelengthFilter.h>
#include <framework/gearbox/Unit.h>
#include <math.h>
#include <iostream>
#include <iomanip>

using namespace std;

namespace Belle2 {

  float TOPWavelengthFilter::getBulkTransmission(double lambda) const
  {
    if (m_transmissions.empty()) return 1; // old payload from DB (filter included in QE)

    double dlam = lambda - m_lambdaFirst;
    if (dlam < 0 or dlam >= (m_transmissions.size() - 1) * m_lambdaStep) return 0;
    unsigned i = int(dlam / m_lambdaStep);
    if (i > m_transmissions.size() - 2) return 0;
    return m_transmissions[i] + (m_transmissions[i + 1] - m_transmissions[i]) / m_lambdaStep * (dlam - i * m_lambdaStep);
  }


  bool TOPWavelengthFilter::isConsistent() const
  {
    if (getName().empty()) return true; // old payload from DB (filter included in QE)

    if (m_lambdaFirst <= 0) return false;
    if (m_lambdaStep <= 0) return false;
    if (m_transmissions.empty()) return false;
    return true;
  }


  void TOPWavelengthFilter::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);
    cout << " first point: " << getLambdaFirst() << " nm";
    cout << ", step: " << getLambdaStep() << " nm" << endl;
    cout << " bulk transmissions: [";
    for (const auto& trE : m_transmissions) cout << setprecision(3) << trE << ", ";
    cout << "]" << endl;
    if (getName().empty()) {
      cout << "(Old geometry payload, filter transmission is included in QE)" << endl;
    }

  }

} // end Belle2 namespace
