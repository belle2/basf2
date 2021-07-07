/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for common T0 calibration with neg. log likelihood minimization
     * method LL
     */
    class TOPCommonT0LLAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPCommonT0LLAlgorithm();

      /** Destructor */
      virtual ~TOPCommonT0LLAlgorithm() {}

      /**
       * Sets minimal common T0 uncertainty to declare this calibration as c_OK
       * @param minError minimal uncertainty [ns]
       */
      void setMinError(double minError) {m_minError = minError;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      // algorithm parameters
      double m_minError = 0.020; /**< minimal commonT0 uncertainty [ns] to declare c_OK */

      // ntuple variables
      int m_expNo = 0; /**< experiment number */
      int m_runNo = 0; /**< first run number */
      int m_runLast = 0; /**< last run number */
      float m_fittedOffset = 0; /**< fitted offset */
      float m_offset = 0; /**< wrap-around of fitted offset (= common T0) */
      float m_offsetError = 0; /**< error on fitted offset */
      float m_errorScaling = 1; /**< factor used for scaling the error */
      int m_numTracks = 0; /**< number of tracks used */
      int m_numEvents = 0; /**< number of events used */
      int m_fitStatus = -1; /**< fit status (0 = OK) */

    };

  } // end namespace TOP
} // end namespace Belle2
