/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <vxd/dbobjects/VXDRotationSolidPar.h>

namespace Belle2 {

  /**
  * The Class for VXD Envelope parameters
  */
  class VXDEnvelopePar: public VXDRotationSolidPar {
  public:
    /** Constructor */
    VXDEnvelopePar(const std::string& name, const std::string& material, const std::string& color,
                   double minPhi, double maxPhi, bool exists):
      VXDRotationSolidPar(name, material, color, minPhi, maxPhi, exists)
    {}
    /** Constructor */
    VXDEnvelopePar():
      VXDRotationSolidPar()
    {}

  private:
    ClassDef(VXDEnvelopePar, 5);  /**< ClassDef, must be the last term before the closing {}*/
  };

} // end of namespace Belle2

