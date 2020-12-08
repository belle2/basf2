/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TOP {

    TOPRecoManager& TOPRecoManager::getInstance()
    {
      static TOPRecoManager instance;
      return instance;
    }

    void TOPRecoManager::set()
    {
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      for (unsigned moduleID = 1; moduleID <= geo->getNumModules(); moduleID++) {
        m_yScanners.push_back(YScanner(moduleID));
        m_inverseRaytracers.push_back(InverseRaytracer(moduleID, m_yScanners.back().getCosTotal()));
        m_fastRaytracers.push_back(FastRaytracer(moduleID));
      }
    }

    const InverseRaytracer* TOPRecoManager::inverseRaytracer(int moduleID)
    {
      if (m_inverseRaytracers.empty()) set();
      unsigned k = moduleID - 1;
      if (k < m_inverseRaytracers.size()) {
        m_inverseRaytracers[k].clear();
        return &m_inverseRaytracers[k];
      }

      B2ERROR("TOPRecoManager::getInverseRaytracer: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }

    const FastRaytracer* TOPRecoManager::fastRaytracer(int moduleID)
    {
      if (m_fastRaytracers.empty()) set();
      unsigned k = moduleID - 1;
      if (k < m_fastRaytracers.size()) {
        m_fastRaytracers[k].clear();
        return &m_fastRaytracers[k];
      }

      B2ERROR("TOPRecoManager::getFastRaytracer: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }

    const YScanner* TOPRecoManager::yScanner(int moduleID)
    {
      if (m_yScanners.empty()) set();
      unsigned k = moduleID - 1;
      if (k < m_yScanners.size()) {
        m_yScanners[k].clear();
        return &m_yScanners[k];
      }

      B2ERROR("TOPRecoManager::getYScanner: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }


  } // namespace TOP
} // namespace Belle2

