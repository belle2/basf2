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

      /**
       * Termination action.
       */
      void finish() override;
    private:
      /**
       * Input array for CDC Raw.
       */
      StoreArray<RawCDC> m_rawCDCs;
      /**
       * Counts of board presense. Note that there is no boardID=0, use the first entry to count events
       */
      int nReadsPerBoard[300];
      /**
       * Minimal threshold of calls for a good board
       */
      int m_minimalCounts{0};
      /**
       *  Report bad boards at the end of the run/at terminate
       */
      void reportBadBoards();
    };
  }
}
