/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <TH1F.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for photon pixel yields aimed for PMT ageing studies and for finding optically decoupled PMT's
     */
    class TOPPhotonYieldsAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPPhotonYieldsAlgorithm();

      /** Destructor */
      virtual ~TOPPhotonYieldsAlgorithm() {}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * Returns equalizing value for alpha ratio.
       * @param bin histogram bin
       * @return equalizing value
       */
      double getEqualizingValue(int bin);

      /**
       * Equalize alpha ratio histogram
       * @param h histogram
       */
      void equalize(TH1F* h);

    };

  } // end namespace TOP
} // end namespace Belle2
