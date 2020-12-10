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

    void TOPRecoManager::setChannelMask(const DBObjPtr<TOPCalChannelMask>& mask,
                                        const TOPAsicMask& asicMask)
    {
      auto& yScanners = TOPRecoManager::getInstance().m_yScanners;
      if (yScanners.empty()) TOPRecoManager::getInstance().set();

      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      for (auto& yScanner : yScanners) {
        auto& pixelMasks = yScanner.pixelMasks();
        int moduleID = pixelMasks.getModuleID();
        unsigned numChannels = pixelMasks.getNumPixels();
        for (unsigned channel = 0; channel < numChannels; channel++) {
          int pixelID = mapper.getPixelID(channel);
          pixelMasks.set(pixelID, mask->isActive(moduleID, channel) and asicMask.isActive(moduleID, channel));
        }
      }

      B2INFO("TOPRecoManager: new channel masks have been passed to reconstruction");
    }

    void TOPRecoManager::setUncalibratedChannelsOff(const DBObjPtr<TOPCalChannelT0>& channelT0)
    {
      auto& yScanners = TOPRecoManager::getInstance().m_yScanners;
      if (yScanners.empty()) TOPRecoManager::getInstance().set();

      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      for (auto& yScanner : yScanners) {
        auto& pixelMasks = yScanner.pixelMasks();
        int moduleID = pixelMasks.getModuleID();
        unsigned numChannels = pixelMasks.getNumPixels();
        for (unsigned channel = 0; channel < numChannels; channel++) {
          if (channelT0->isCalibrated(moduleID, channel)) continue;
          int pixelID = mapper.getPixelID(channel);
          pixelMasks.setPixelOff(pixelID);
        }
      }

      B2INFO("TOPRecoManager: channelT0-uncalibrated channels have been masked off");
    }

    void TOPRecoManager::setUncalibratedChannelsOff(const DBObjPtr<TOPCalTimebase>& timebase)
    {
      auto& yScanners = TOPRecoManager::getInstance().m_yScanners;
      if (yScanners.empty()) TOPRecoManager::getInstance().set();

      const auto& ch_mapper = TOPGeometryPar::Instance()->getChannelMapper();
      const auto& fe_mapper = TOPGeometryPar::Instance()->getFrontEndMapper();
      for (auto& yScanner : yScanners) {
        auto& pixelMasks = yScanner.pixelMasks();
        int moduleID = pixelMasks.getModuleID();
        unsigned numChannels = pixelMasks.getNumPixels();
        for (unsigned channel = 0; channel < numChannels; channel++) {
          const auto* fe = fe_mapper.getMap(moduleID, channel / 128);
          if (not fe) {
            B2ERROR("TOPRecoManager::setUncalibratedChannelsOff no front-end map found");
            continue;
          }
          auto scrodID = fe->getScrodID();
          const auto* sampleTimes = timebase->getSampleTimes(scrodID, channel);
          if (sampleTimes->isCalibrated()) continue;
          int pixelID = ch_mapper.getPixelID(channel);
          pixelMasks.setPixelOff(pixelID);
        }
      }

      B2INFO("TOPRecoManager: timebase-uncalibrated channels have been masked off");
    }

    void TOPRecoManager::setChannelEffi()
    {
      auto& yScanners = TOPRecoManager::getInstance().m_yScanners;
      if (yScanners.empty()) TOPRecoManager::getInstance().set();

      for (auto& yScanner : yScanners) {
        auto& pixelEfficiencies = yScanner.pixelEfficiencies();
        int moduleID = pixelEfficiencies.getModuleID();
        int numPixels = pixelEfficiencies.getNumPixels();
        for (int pixelID = 1; pixelID <= numPixels; pixelID++) {
          double effi = TOPGeometryPar::Instance()->getRelativePixelEfficiency(moduleID, pixelID);
          pixelEfficiencies.set(pixelID, effi);
        }
      }

      B2INFO("TOPRecoManager: new relative pixel efficiencies have been passed to reconstruction");
    }

  } // namespace TOP
} // namespace Belle2

