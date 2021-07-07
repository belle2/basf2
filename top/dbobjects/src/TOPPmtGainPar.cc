/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPPmtGainPar.h>
#include <iostream>
#include <iomanip>

using namespace std;

namespace Belle2 {

  void TOPPmtGainPar::print() const
  {
    cout << "PMT gain parameters:" << endl;
    cout << " Serial number: " << getSerialNumber() << endl;
    cout << " Nominal HV: " << getNominalHV() << " V" << endl;
    for (unsigned channel = 1; channel <= c_NumPmtPixels; channel++) {
      cout << setw(4) << channel;
      cout << setw(12) << getConstant(channel);
      cout << setw(12) << getSlope(channel);
      cout << setw(12) << getRatio(channel);
      cout << setw(12) << getGain0(channel, getNominalHV());
      cout << endl;
    }
    cout << endl;
  }

} // end Belle2 namespace
