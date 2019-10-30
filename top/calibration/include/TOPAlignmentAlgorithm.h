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
     * which is in fact run in the collector modules.
     */
    class TOPAlignmentAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPAlignmentAlgorithm();

      /** Destructor */
      virtual ~TOPAlignmentAlgorithm() {}

      /**
       * Sets required precision of displacements to declare calibration as c_OK
       * @param precision required precision
       */
      void setSpatialPrecision(double precision) {m_spatialPrecision = precision;}

      /**
       * Sets required precision of rotation angles to declare calibration as c_OK
       * @param precision required precision
       */
      void setAngularPrecision(double precision) {m_angularPrecision = precision;}

    private:

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      // algorithm parameters
      double m_spatialPrecision = 0.1; /**< precision of displacements for c_OK */
      double m_angularPrecision = 0.001; /**< precision of rotation angles for c_OK */

      // input tree variables
      int m_moduleID = 0; /**< module ID */
      int m_iter = 0;  /**< iteration counter */
      int m_ntrk = 0;  /**< number of tracks used */
      int m_errorCode = 0;  /**< error code of the alignment procedure */
      std::vector<float>* m_vAlignPars = 0;     /**< alignment parameters */
      std::vector<float>* m_vAlignParsErr = 0;  /**< error on alignment parameters */
      bool m_valid = false;  /**< true if alignment parameters are valid */
      TBranch* m_bAlignPars = 0; /**< branch of alignment parameters */
      TBranch* m_bAlignParsErr = 0; /**< branch of error on alignment parameters */

    };

  } // end namespace TOP
} // end namespace Belle2
