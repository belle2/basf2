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
#include <calibration/CalibrationAlgorithm.h>
#include <TH1F.h>
#include "vector"
#include "string"
namespace Belle2 {
  namespace CDC {
    /**
     * Algorithm class for crude T0 calibration.
     */
    class CrudeT0CalibrationAlgorithm: public CalibrationAlgorithm {
    public:
      /**
       * Constructor.
       */
      CrudeT0CalibrationAlgorithm();
      /**
       * Destructor.
       */
      virtual ~CrudeT0CalibrationAlgorithm() {}

    protected:
      /// Run algo on data
      virtual EResult calibrate();
      ///create histo for each channel
      virtual void createHisto();
      /// write outut or store db
      virtual void write();
    private:

      double m_t0b[300];    /**< T0 for each board*/
      double m_t0[56][400]; /**< T0 of each channel*/
      bool m_flag[56][400] = {{false}}; /**< flag =1 for good, =0 for low statistic or bad fit */

      TH1D* m_hTDC[56][400];    /**< TDC distribution histo. */
      TH1D* m_hTDCBoard[300];   /**< T0 distribution of each board. */
      TH1D* m_hT0All;           /**< T0 distribution of all channel. */
      unsigned short m_tdcMin = 4500; /**< minimum of TDC hist for fitting */
      unsigned short m_tdcMax = 5000; /**< maximum of TDC hist for fitting */
      float m_initT0 = 4887.;   /**< Common initial T0 for fitting */
      unsigned short m_minEntries = 100; /**< minimum entries required by histo. */
      double m_zOffset = 0.0;        /**< z offset for calculate prop time, it is position of trigger counter, */
      bool m_cosmic = true;    /**< for cosmic case, tof of upper sector will be negative*/

    };
  }
}

