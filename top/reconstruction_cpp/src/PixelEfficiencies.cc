/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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



