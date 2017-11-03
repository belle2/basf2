/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/dbobjects/TOPGeoPMTArrayDisplacement.h>
#include <iostream>

using namespace std;

namespace Belle2 {

  void TOPGeoPMTArrayDisplacement::print(const std::string& title) const
  {
    TOPGeoBase::print(title);
    cout << " translation vector: (" << getX() << ", " << getY() << ")"
         << " " << s_unitName << endl;
    cout << " rotation angle: " << m_alpha << endl;
  }

} // end Belle2 namespace
