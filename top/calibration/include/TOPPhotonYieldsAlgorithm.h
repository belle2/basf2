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
     * Algorithm for photon pixel yields aimed for PMT ageing studies, for calibration of relative pixel efficiencies
     * and for finding optically decoupled PMT's
     */
    class TOPPhotonYieldsAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPPhotonYieldsAlgorithm();

      /** Destructor */
      virtual ~TOPPhotonYieldsAlgorithm() {}

      /**
       * Sets maximum RQE required for good calibration
       * @param rqe value to be set
       */
      void setMaxRQE(double rqe) {m_maxRQE = rqe;}

      /**
       * Sets maximum RQE uncertainty required for good calibration
       * @param err value to be set
       */
      void setMaxErrorRQE(double err) {m_maxErrorRQE = err;}

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
      double getEqualizingValue(int bin) const;

      /**
       * Equalize alpha ratio histogram
       * @param h histogram
       */
      void equalize(TH1F* h) const;

      /**
       * Returns photon yield of nominal QE for given slot and pixel
       * @param slot slot ID (1-based)
       * @param row pixel row (0-based)
       * @param col pixel column (0-based)
       * @return number of photons per muon which comes from di-muon events
       */
      double getNominalYield(int slot, int row, int col) const;

      /**
       * Returns photon yield correction factor according to muon local-z distribution
       * @param h_mu histogram of muon local-z distribution (must have 100 bins from -130 to 130 cm)
       * @param row pixel row (0-based)
       * @return correction factor
       */
      double getMuonCorrection(const TH1F* h_mu, int row) const;

      double m_maxRQE = 2.0; /**< maximal RQE required for good calibration */
      double m_maxErrorRQE = 0.1; /**< maximal error on RQE required for good calibration */

    };

  } // end namespace TOP
} // end namespace Belle2
