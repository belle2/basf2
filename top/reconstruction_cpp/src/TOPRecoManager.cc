/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/TOPRecoManager.h>
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
        m_backgroundPDFs.push_back(BackgroundPDF(moduleID));
      }
    }

    const InverseRaytracer* TOPRecoManager::getInverseRaytracer(int moduleID)
    {
      const auto& collection = getInstance().inverseRaytracers();
      unsigned k = moduleID - 1;
      if (k < collection.size()) {
        collection[k].clear();
        return &collection[k];
      }

      B2ERROR("TOPRecoManager::getInverseRaytracer: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }

    const FastRaytracer* TOPRecoManager::getFastRaytracer(int moduleID)
    {
      const auto& collection = getInstance().fastRaytracers();
      unsigned k = moduleID - 1;
      if (k < collection.size()) {
        collection[k].clear();
        return &collection[k];
      }

      B2ERROR("TOPRecoManager::getFastRaytracer: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }

    const YScanner* TOPRecoManager::getYScanner(int moduleID)
    {
      const auto& collection = getInstance().yScanners();
      unsigned k = moduleID - 1;
      if (k < collection.size()) {
        collection[k].clear();
        return &collection[k];
      }

      B2ERROR("TOPRecoManager::getYScanner: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }


    const BackgroundPDF* TOPRecoManager::getBackgroundPDF(int moduleID)
    {
      const auto& collection = getInstance().backgroundPDFs();
      unsigned k = moduleID - 1;
      if (k < collection.size()) {
        return &collection[k];
      }

      B2ERROR("TOPRecoManager::getBackgroundPDF: invalid moduleID" << LogVar("moduleID", moduleID));
      return 0;
    }

    void TOPRecoManager::setChannelMask(const DBObjPtr<TOPCalChannelMask>& mask,
                                        const TOPAsicMask& asicMask)
    {
      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      for (auto& yScanner : getInstance().yScanners()) {
        auto& pixelMasks = yScanner.pixelMasks();
        int moduleID = pixelMasks.getModuleID();
        unsigned numChannels = pixelMasks.getNumPixels();
        for (unsigned channel = 0; channel < numChannels; channel++) {
          int pixelID = mapper.getPixelID(channel);
          pixelMasks.set(pixelID, mask->isActive(moduleID, channel) and asicMask.isActive(moduleID, channel));
        }
      }
      getInstance().m_redoBkg = true;

      B2INFO("TOPRecoManager: new channel masks have been passed to reconstruction");
    }

    void TOPRecoManager::setUncalibratedChannelsOff(const DBObjPtr<TOPCalChannelT0>& channelT0)
    {
      const auto& mapper = TOPGeometryPar::Instance()->getChannelMapper();
      for (auto& yScanner : getInstance().yScanners()) {
        auto& pixelMasks = yScanner.pixelMasks();
        int moduleID = pixelMasks.getModuleID();
        unsigned numChannels = pixelMasks.getNumPixels();
        for (unsigned channel = 0; channel < numChannels; channel++) {
          if (channelT0->isCalibrated(moduleID, channel)) continue;
          int pixelID = mapper.getPixelID(channel);
          pixelMasks.setPixelOff(pixelID);
        }
      }
      getInstance().m_redoBkg = true;

      B2INFO("TOPRecoManager: channelT0-uncalibrated channels have been masked off");
    }

    void TOPRecoManager::setUncalibratedChannelsOff(const DBObjPtr<TOPCalTimebase>& timebase)
    {
      const auto& ch_mapper = TOPGeometryPar::Instance()->getChannelMapper();
      const auto& fe_mapper = TOPGeometryPar::Instance()->getFrontEndMapper();
      for (auto& yScanner : getInstance().yScanners()) {
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
      getInstance().m_redoBkg = true;

      B2INFO("TOPRecoManager: timebase-uncalibrated channels have been masked off");
    }

    void TOPRecoManager::setChannelEffi()
    {
      const double minEffi = 0.1;
      const double maxEffi = 2.0;
      for (auto& yScanner : getInstance().yScanners()) {
        auto& pixelEfficiencies = yScanner.pixelEfficiencies();
        int moduleID = pixelEfficiencies.getModuleID();
        int numPixels = pixelEfficiencies.getNumPixels();
        for (int pixelID = 1; pixelID <= numPixels; pixelID++) {
          double effi = TOPGeometryPar::Instance()->getRelativePixelEfficiency(moduleID, pixelID);
          if (effi < minEffi) {
            B2WARNING("TOPRecoManager::setChannelEffi: relative pixel efficiency found rather small"
                      << LogVar("slot", moduleID)
                      << LogVar("pixel", pixelID)
                      << LogVar("efficiency", effi)
                      << " -> will set it to " << minEffi);
            effi = minEffi;
          } else if (effi > maxEffi) {
            B2WARNING("TOPRecoManager::setChannelEffi: relative pixel efficiency found rather large"
                      << LogVar("slot", moduleID)
                      << LogVar("pixel", pixelID)
                      << LogVar("efficiency", effi)
                      << " -> will set it to " << maxEffi);
            effi = maxEffi;
          }
          pixelEfficiencies.set(pixelID, effi);
        }
      }
      getInstance().m_redoBkg = true;

      B2INFO("TOPRecoManager: new relative pixel efficiencies have been passed to reconstruction");
    }

    void TOPRecoManager::setMirrorCenter(int moduleID, double xc, double yc)
    {
      auto& inverseRaytracers = getInstance().inverseRaytracers();
      auto& fastRaytracers = getInstance().fastRaytracers();
      auto& yScanners = getInstance().yScanners();
      unsigned k = moduleID - 1;
      if (k < yScanners.size()) {
        inverseRaytracers[k].setMirrorCenter(xc, yc);
        fastRaytracers[k].setMirrorCenter(xc, yc);
        yScanners[k].setMirrorCenter(xc, yc);
        return;
      }

      B2ERROR("TOPRecoManager::setMirrorCenter: invalid moduleID" << LogVar("moduleID", moduleID));
    }


  } // namespace TOP
} // namespace Belle2

