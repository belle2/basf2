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
     * Algorithm for module T0 calibration with neg. log likelihood minimization.
     * Method LL
     */
    class TOPModuleT0LLAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPModuleT0LLAlgorithm();

      /** Destructor */
      virtual ~TOPModuleT0LLAlgorithm() {}

      /**
       * Sets minimal module T0 uncertainty to declare this calibration as c_OK
       * @param minError minimal uncertainty [ns]
       */
      void setMinError(double minError) {m_minError = minError;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      // algorithm parameters
      double m_minError = 0.020; /**< minimal moduleT0 uncertainty [ns] to declare c_OK */

    };

  } // end namespace TOP
} // end namespace Belle2
