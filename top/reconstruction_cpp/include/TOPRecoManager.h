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

    };

  } // namespace TOP
} // namespace Belle2

