/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
