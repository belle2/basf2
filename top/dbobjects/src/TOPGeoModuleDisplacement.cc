/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoModuleDisplacement.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  void TOPGeoModuleDisplacement::print(const std::string& title) const
  {
    TOPGeoBase::print(title);
    cout << " translation vector: (" << getX() << ", "
         << getY() << ", " << getZ() << ")" << " " << s_unitName << endl;
    cout << " rotation angles: ";
    cout << getAlpha() << " ";
    cout << getBeta() << " ";
    cout << getGamma() << " ";
    cout << endl;
  }

} // end Belle2 namespace
