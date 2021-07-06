/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPWavelengthFilter.h>
#include <iostream>
#include <iomanip>

using namespace std;

namespace Belle2 {

  float TOPWavelengthFilter::getBulkTransmittance(double lambda) const
  {
    if (m_transmittances.empty()) return 1; // old payload from DB (filter included in QE)

    double dlam = lambda - m_lambdaFirst;
    if (dlam < 0) return 0;
    if (dlam > (m_transmittances.size() - 1) * m_lambdaStep) return m_transmittances.back();
    unsigned i = int(dlam / m_lambdaStep);
    if (i > m_transmittances.size() - 2) return m_transmittances.back();
    return m_transmittances[i] + (m_transmittances[i + 1] - m_transmittances[i]) / m_lambdaStep * (dlam - i * m_lambdaStep);
  }


  bool TOPWavelengthFilter::isConsistent() const
  {
    if (getName().empty()) return true; // old payload from DB (filter included in QE)

    if (m_lambdaFirst <= 0) return false;
    if (m_lambdaStep <= 0) return false;
    if (m_transmittances.empty()) return false;
    return true;
  }


  void TOPWavelengthFilter::print(const std::string& title) const
  {
    TOPGeoBase::printUnderlined(title);
    cout << " first point: " << getLambdaFirst() << " nm";
    cout << ", step: " << getLambdaStep() << " nm" << endl;
    cout << " bulk transmittances: [";
    for (const auto& trE : m_transmittances) cout << setprecision(3) << trE << ", ";
    cout << "]" << endl;
    if (getName().empty()) {
      cout << "(Old geometry payload, filter transmittance is included in QE)" << endl;
    }

  }

} // end Belle2 namespace
