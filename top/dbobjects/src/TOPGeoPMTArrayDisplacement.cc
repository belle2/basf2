/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
