/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/PixelEfficiencies.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TOP {

    PixelEfficiencies::PixelEfficiencies(int moduleID): m_moduleID(moduleID)
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      if (not geo->isModuleIDValid(moduleID)) {
        B2FATAL("TOP::PixelEfficiencies: invalid slot number, moduleID = " << moduleID);
        return;
      }
      int numPixels = geo->getModule(moduleID).getPMTArray().getNumPixels();
      m_efficiencies.resize(numPixels, 1.0);
    }

  } // TOP
} // Belle2



