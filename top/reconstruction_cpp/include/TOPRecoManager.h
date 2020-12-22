/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction_cpp/InverseRaytracer.h>
#include <top/reconstruction_cpp/FastRaytracer.h>
#include <top/reconstruction_cpp/YScanner.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/database/DBObjPtr.h>
#include <top/dataobjects/TOPAsicMask.h>

#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Singleton class providing pre-constructed reconstruction objects
     */
    class TOPRecoManager {

    public:

      /**
       * Returns inverse ray-tracer of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to inverse ray-tracer or null pointer if moduleID is not valid
       */
      static const InverseRaytracer* getInverseRaytracer(int moduleID)
      {
        return TOPRecoManager::getInstance().inverseRaytracer(moduleID);
      }

      /**
       * Returns fast ray-tracer of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to fast ray-tracer or null pointer if moduleID is not valid
       */
      static const FastRaytracer* getFastRaytracer(int moduleID)
      {
        return TOPRecoManager::getInstance().fastRaytracer(moduleID);
      }

      /**
       * Returns y-scanner of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to y-scanner or null pointer if moduleID is not valid
       */
      static const YScanner* getYScanner(int moduleID)
      {
        return TOPRecoManager::getInstance().yScanner(moduleID);
      }

      /**
       * Returns time window lower edge
       * @return time window lower edge
       */
      static double getMinTime() {return TOPRecoManager::getInstance().m_minTime;}

      /**
       * Returns time window upper edge
       * @return time window upper edge
       */
      static double getMaxTime() {return TOPRecoManager::getInstance().m_maxTime;}

      /**
       * Sets time window for likelihood determination.
       * This function must be called per each event to prevent confusions
       * when different modules set time window differently.
       * Window edges must not exceed those used during data taking or in simulation.
       * @param minTime lower edge
       * @param maxTime upper edge
       */
      static void setTimeWindow(double minTime, double maxTime)
      {
        TOPRecoManager::getInstance().m_minTime = minTime;
        TOPRecoManager::getInstance().m_maxTime = maxTime;
      }

      /**
       * Sets channel masks
       * @param mask channel mask
       * @param asicMask masked asics
       */
      static void setChannelMask(const DBObjPtr<TOPCalChannelMask>& mask,
                                 const TOPAsicMask& asicMask);

      /**
       * Sets uncalibrated channels off
       * @param channelT0 channel T0 calibration
       */
      static void setUncalibratedChannelsOff(const DBObjPtr<TOPCalChannelT0>& channelT0);

      /**
       * Sets uncalibrated channels off
       * @param timebase timebase calibration
       */
      static void setUncalibratedChannelsOff(const DBObjPtr<TOPCalTimebase>& timebase);

      /**
       * Sets relative efficiencies of pixels
       */
      static void setChannelEffi();

      /**
       * Returns instance of the class
       * @return class instance
       */
      static TOPRecoManager& getInstance();

    private:

      /** Singleton: private constructor */
      TOPRecoManager() = default;
      /** Singleton: no copy constructor */
      TOPRecoManager(TOPRecoManager&) = delete;
      /** Singleton: no assignment operator */
      TOPRecoManager& operator=(const TOPRecoManager&) = delete;
      /** Singleton: private destructor */
      ~TOPRecoManager() = default;

      /** Sets the reconstruction objects in class memory */
      void set();

      /**
       * Returns inverse ray-tracer of a given module
       * @param moduleID slot ID
       * @return inverse ray-tracer or null pointer for invalid module
       */
      const InverseRaytracer* inverseRaytracer(int moduleID);

      /**
       * Returns fast ray-tracer of a given module
       * @param moduleID slot ID
       * @return fast ray-tracer or null pointer for invalid moduleID
       */
      const FastRaytracer* fastRaytracer(int moduleID);

      /**
       * Returns y-scanner of a given module
       * @param moduleID slot ID
       * @return y-scanner or null pointer for invalid moduleID
       */
      const YScanner* yScanner(int moduleID);

      std::vector<InverseRaytracer> m_inverseRaytracers; /**< collection of inverse raytracers */
      std::vector<FastRaytracer> m_fastRaytracers; /**< collection of fast raytracers */
      std::vector<YScanner> m_yScanners; /**< collection of y-scanners */
      double m_minTime = -20; /**< time window lower edge */
      double m_maxTime = 75; /**< time window upper edge */

    };

  } // namespace TOP
} // namespace Belle2

