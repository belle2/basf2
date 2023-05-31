/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPGeoModuleDisplacement.h>
#include <iostream>
#include <Math/RotationX.h>
#include <Math/RotationY.h>
#include <Math/RotationZ.h>
#include <Math/Translation3D.h>

using namespace std;
using namespace ROOT::Math;

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

  Transform3D TOPGeoModuleDisplacement::getTransformation() const
  {
    RotationX Rx(m_alpha);
    RotationY Ry(m_beta);
    RotationZ Rz(m_gamma);
    Translation3D t(m_x, m_y, m_z);
    return Transform3D(Rz * Ry * Rx, t);
  }

} // end Belle2 namespace
