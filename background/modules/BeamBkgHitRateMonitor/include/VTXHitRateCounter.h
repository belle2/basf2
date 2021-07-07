/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Benjamin Schwenker                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <vtx/dataobjects/VTXDigit.h>
#include <vtx/dataobjects/VTXCluster.h>
#include <vxd/dataobjects/VxdID.h>
#include <vtx/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <TTree.h>
#include <map>

#include <framework/gearbox/Unit.h>

namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of VTX
     */
    class VTXHitRateCounter: public HitRateBase {

    public:

      /**
       * tree structure
       */
      struct TreeStruct {
        float averageRate = 0; /**< total detector average hit rate */
        float averageRateCluster = 0; /**< total detector average cluster rate */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */
      };

      /**
       * Constructor
       */
      VTXHitRateCounter()
      {}

      /**
       * Class initializer: set branch addresses and other staf
       * @param tree a valid TTree pointer
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'
       */
      virtual void clear() override;

      /**
       * Accumulate hits
       * @param timeStamp time stamp
       */
      virtual void accumulate(unsigned timeStamp) override;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       * @param timeStamp time stamp
       */
      virtual void normalize(unsigned timeStamp) override;

    private:

      /**
       * Get VTX::SensorInfo
       */
      // cppcheck-suppress unusedPrivateFunction
      inline const VTX::SensorInfo& getInfo(VxdID sensorID) const;

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<VTXDigit> m_digits;  /**< collection of digits */
      StoreArray<VTXCluster> m_clusters;  /**< collection of clusters */

    };

    inline const VTX::SensorInfo& VTXHitRateCounter::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const VTX::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

  } // Background namespace
} // Belle2 namespace
