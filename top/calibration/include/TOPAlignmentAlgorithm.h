/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <vector>
#include <map>

namespace Belle2 {
  namespace TOP {

    /**
     * Algorithm for geometrical alignment of TOP modules with dimuons or Bhabhas.
     * This class just collects the results of iterative alignment,
     * which runs in the collector modules.
     */
    class TOPAlignmentAlgorithm : public CalibrationAlgorithm {
    public:

      /** Constructor */
      TOPAlignmentAlgorithm();

      /** Destructor */
      virtual ~TOPAlignmentAlgorithm() {}

      /**
       * Sets required precision of displacements to declare calibration as c_OK
       * @param precision required precision [cm]
       */
      void setSpatialPrecision(double precision) {m_spatialPrecision = precision;}

      /**
       * Sets required precision of rotation angles to declare calibration as c_OK
       * @param precision required precision [rad]
       */
      void setAngularPrecision(double precision) {m_angularPrecision = precision;}

    private:

      /**
       * data structure
       */
      struct AlignData {
        int iter = 0;  /**< iteration counter */
        int ntrk = 0;  /**< number of tracks used */
        std::vector<float> alignPars;  /**< alignment parameters */
        std::vector<float> alignErrs;  /**< uncertainties on alignment parameters */
        bool valid = false;  /**< true if alignment parameters are valid */
        /**
         * Merge another data structure to this one.
         * Implements weighted average (least square fit) of alignment parameters.
         * Function finalize() must be called after all data structures are added.
         */
        void add(const AlignData& data);
        /**
         * Calculate weighted averages and rescale errors
         * @param scaleFact scale factor for errors
         */
        void finalize(double scaleFact);
      };

      /**
       * algorithm implementation
       */
      virtual EResult calibrate() final;

      /**
       * merge subsamples and rescale errors
       */
      void mergeData();

      // algorithm parameters
      double m_spatialPrecision = 0.1; /**< required precision of displacements */
      double m_angularPrecision = 0.001; /**< required precision of rotation angles */

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

      // maps of moduleID and AlignData
      std::multimap<int, AlignData> m_inputData; /**< input from ntuples */
      std::map<int, AlignData> m_mergedData; /**< merged subsamples */

    };

  } // end namespace TOP
} // end namespace Belle2
