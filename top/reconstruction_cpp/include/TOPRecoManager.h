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
#include <top/reconstruction_cpp/BackgroundPDF.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/database/DBObjPtr.h>
#include <top/dataobjects/TOPAsicMask.h>
#include <top/geometry/TOPGeometryPar.h>

#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Singleton class providing pre-constructed reconstruction objects
     */
    class TOPRecoManager {

    public:

      /**
       * Returns instance of the class
       * @return class instance
       */
      static TOPRecoManager& getInstance();

      /**
       * Returns inverse ray-tracer of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to inverse ray-tracer or null pointer if moduleID is not valid
       */
      static const InverseRaytracer* getInverseRaytracer(int moduleID);

      /**
       * Returns fast ray-tracer of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to fast ray-tracer or null pointer if moduleID is not valid
       */
      static const FastRaytracer* getFastRaytracer(int moduleID);

      /**
       * Returns y-scanner of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to y-scanner or null pointer if moduleID is not valid
       */
      static const YScanner* getYScanner(int moduleID);

      /**
       * Returns background PDF of a given module
       * @param moduleID slot ID (1-based)
       * @return pointer to background PDF or null pointer if moduleID is not valid
       */
      static const BackgroundPDF* getBackgroundPDF(int moduleID);

      /**
       * Returns background PDF's of all modules
       * @return collection of background PDF's (index = moduleID - 1)
       */
      static const std::vector<BackgroundPDF>& getBackgroundPDFs() {return getInstance().backgroundPDFs();}

      /**
       * Returns time window lower edge
       * @return time window lower edge
       */
      static double getMinTime()
      {
        if (getInstance().m_minTime < getInstance().m_maxTime) {
          return getInstance().m_minTime;
        }
        return TOPGeometryPar::Instance()->getGeometry()->getNominalTDC().getTimeMin();
      }

      /**
       * Returns time window upper edge
       * @return time window upper edge
       */
      static double getMaxTime()
      {
        if (getInstance().m_minTime < getInstance().m_maxTime) {
          return getInstance().m_maxTime;
        }
        return TOPGeometryPar::Instance()->getGeometry()->getNominalTDC().getTimeMax();
      }

      /**
       * Returns size of time window
       * @return size of time window
       */
      static double getTimeWindowSize() {return getMaxTime() - getMinTime();}

      /**
       * Sets time window
       * @param minTime lower edge
       * @param maxTime upper edge
       */
      static void setTimeWindow(double minTime, double maxTime)
      {
        getInstance().m_minTime = minTime;
        getInstance().m_maxTime = maxTime;
      }

      /**
       * Sets default time window (functions getMinTime(), getMaxTime() will then return default values from DB)
       */
      static void setDefaultTimeWindow()
      {
        getInstance().m_minTime = 0;
        getInstance().m_maxTime = 0;
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

    private:

      /** Singleton: private constructor */
      TOPRecoManager() = default;
      /** Singleton: no copy constructor */
      TOPRecoManager(TOPRecoManager&) = delete;
      /** Singleton: no assignment operator */
      TOPRecoManager& operator=(const TOPRecoManager&) = delete;
      /** Singleton: private destructor */
      ~TOPRecoManager() = default;

      /** Sets the reconstruction object collections */
      void set();

      /**
       * Interface to inverse ray-tracers of all modules.
       * Any accesses to underlying collection must be made with this method.
       * @return collection of inverse ray-tracers (index = moduleID - 1)
       */
      std::vector<InverseRaytracer>& inverseRaytracers();

      /**
       * Interface to fast ray-tracers of all modules.
       * Any accesses to underlying collection must be made with this method.
       * @return collection of fast ray-tracers (index = moduleID - 1)
       */
      std::vector<FastRaytracer>& fastRaytracers();

      /**
       * Interface to y-scanners of all modules.
       * Any accesses to underlying collection must be made with this method.
       * @return collection of y-scanners (index = moduleID - 1)
       */
      std::vector<YScanner>& yScanners();

      /**
       * Interface to background PDF's of all modules.
       * Any accesses to underlying collection must be made with this method.
       * @return collection of background PDF's (index = moduleID - 1)
       */
      std::vector<BackgroundPDF>& backgroundPDFs();

      std::vector<InverseRaytracer> m_inverseRaytracers; /**< collection of inverse raytracers */
      std::vector<FastRaytracer> m_fastRaytracers; /**< collection of fast raytracers */
      std::vector<YScanner> m_yScanners; /**< collection of y-scanners */
      std::vector<BackgroundPDF> m_backgroundPDFs; /**< collection of background PDF's */
      double m_minTime = 0; /**< time window lower edge */
      double m_maxTime = 0; /**< time window upper edge */
      bool m_redoBkg = false; /**< flag to signal whether backgroundPDF has to be redone */

    };

    //--- inline functions ------------------------------------------------------------

    inline std::vector<InverseRaytracer>& TOPRecoManager::inverseRaytracers()
    {
      if (m_inverseRaytracers.empty()) set();
      return m_inverseRaytracers;
    }

    inline std::vector<FastRaytracer>& TOPRecoManager::fastRaytracers()
    {
      if (m_fastRaytracers.empty()) set();
      return m_fastRaytracers;
    }

    inline std::vector<YScanner>& TOPRecoManager::yScanners()
    {
      if (m_yScanners.empty()) set();
      return m_yScanners;
    }

    inline std::vector<BackgroundPDF>& TOPRecoManager::backgroundPDFs()
    {
      if (m_backgroundPDFs.empty()) set();
      if (m_redoBkg) {
        for (auto& pdf : m_backgroundPDFs) pdf.set();
        m_redoBkg = false;
      }
      return m_backgroundPDFs;
    }

  } // namespace TOP
} // namespace Belle2

