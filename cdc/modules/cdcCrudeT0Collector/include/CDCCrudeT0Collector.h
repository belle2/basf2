/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>

#include <string>
namespace Belle2 {

  namespace CDC {
    /**
     * Collector for crude t0.
     */
    class CDCCrudeT0CollectorModule : public CalibrationCollectorModule {
    public:

      /**
       * Constructor.
       */
      CDCCrudeT0CollectorModule();

      virtual void prepare();
      virtual void collect();
      virtual void startRun();
      virtual void closeRun();
      virtual void finish();

    private:
      unsigned short m_lay;  /**< Layer ID */
      unsigned short m_wire; /**< Wire ID */
      unsigned short m_tdc;  /**< TDC count */
      StoreArray<CDCHit> m_cdcHits; /**< CDCHit array */
      unsigned short m_adcMin = 1;    /**< ADC cut to reject noise. */
      /**
       * Describe the process.
       */
      void describeProcess(std::string functionName);

    };
  }
}
