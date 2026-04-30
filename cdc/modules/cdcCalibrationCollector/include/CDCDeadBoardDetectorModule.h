/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawCDC.h>

namespace Belle2 {

  namespace CDC {

    /**
     * CDCDeadBoardDetectorModule: The CDC Raw Hits Decoder.
     */

    class CDCDeadBoardDetectorModule : public CalibrationCollectorModule {

    public:
      /**
       * Constructor of the module.
       */
      CDCDeadBoardDetectorModule();

      /**
       * Destructor of the module.
       */
      virtual ~CDCDeadBoardDetectorModule();

      /**
       * Initializes the Module.
       */
      void prepare() override;

      /**
       * Event action (main routine).
       *
       */
      void collect() override;

    private:
      /**
       * Input array for CDC Raw.
       */
      StoreArray<RawCDC> m_rawCDCs;
    };
  }
}
