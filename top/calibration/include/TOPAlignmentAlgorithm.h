/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for geometrical alignment of a TOP module with dimuons or Bhabhas.
     * This class just collects the results of iterative alignment,
     * which is in fact run in the collector module.
     */
    class TOPAlignmentAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPAlignmentAlgorithm();

      /** Destructor */
      virtual ~TOPAlignmentAlgorithm() {}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      // input tree variables
      int m_moduleID = 0; /**< module ID */
      int m_iter = 0;  /**< iteration counter */
      int m_ntrk = 0;  /**< number of tracks used */
      int m_errorCode = 0;  /**< error code of the alignment procedure */
      std::vector<float> m_vAlignPars;     /**< alignment parameters */
      std::vector<float> m_vAlignParsErr;  /**< error on alignment parameters */
      bool m_valid = false;  /**< true if alignment parameters are valid */

    };

  } // end namespace TOP
} // end namespace Belle2
