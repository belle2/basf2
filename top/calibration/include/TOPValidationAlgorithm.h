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

      enum {c_numModules = TOP::ValidationTreeStruct::c_numModules};  /**< number of modules */

      ValidationTreeStruct m_treeEntry;  /**< input tree entry */

    };

  } // end namespace TOP
} // end namespace Belle2
