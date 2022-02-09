/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <top/calibration/ValidationTreeStruct.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for automatic validation of the calibration
     */
    class TOPValidationAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPValidationAlgorithm();

      /** Destructor */
      virtual ~TOPValidationAlgorithm() {}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;


      ValidationTreeStruct m_inputEntry;  /**< input tree entry */
      ValidationTreeStruct m_outputEntry; /**< output tree entry, per run merged entries of input tree */

    };

  } // end namespace TOP
} // end namespace Belle2
