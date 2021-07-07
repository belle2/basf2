/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TTree.h>

namespace Belle2 {
  namespace Background {

    /**
     * Abstract base class for monitoring beam background hit rates
     * All the monitoring classes must inherit from this one
     */
    class HitRateBase {

    public:

      /**
       * virtual destructor
       */
      virtual ~HitRateBase()
      {}

      /**
       * Class initializer: set branch address
       * @param tree TTree pointer
       */
      virtual void initialize(TTree* tree) = 0;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'
       */
      virtual void clear() = 0;

      /**
       * Accumulate hits
       * @param timeStamp time stamp
       */
      virtual void accumulate(unsigned timeStamp) = 0;

      /**
       * Normalize accumulated hits (e.g. transform to rates) and prepare for TTree::Fill
       * @param timeStamp time stamp
       */
      virtual void normalize(unsigned timeStamp) = 0;

    };

  } // Background namespace
} // Belle2 namespace
