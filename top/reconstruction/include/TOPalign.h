/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction/TOPtrack.h>
#include <framework/gearbox/Const.h>
#include <vector>

namespace Belle2 {
  namespace TOP {

    /**
     * Alignment of a TOP module: core of the method is coded in fortran
     */
    class TOPalign {
    public:

      /**
       * Number of free parameters
       */
      enum {c_numPar = 7};

      /**
       * Constructor
       * @param moduleID module ID
       * @param stepPosition step size for translations
       * @param stepAngle step size for rotations
       * @param stepTime step size for T0
       */
      TOPalign(int moduleID,
               double stepPosition = 1.0,
               double stepAngle = 0.01,
               double stepTime = 0.05);

      /**
       * Clear data list
       */
      static void clearData();

      /**
       * Add data
       * @param moduleID module ID
       * @param pixelID pixel ID (e.g. software channel, 1-based)
       * @param time TBC and local T0 corrected time in [ns]
       * @param timeError time uncertainty in [ns]
       * @return data size (or 0 on error)
       */
      static int addData(int moduleID, int pixelID, double time, double timeError);

      /**
       * Sets expected photon yields
       * @param bkgPerModule estimation for minimal number of background hits
       * @param scaleN0 scale factor for figure-of-merit N0
       */
      static void setPhotonYields(double bkgPerModule, double scaleN0 = 1);

      /**
       * Sets initial values of parameters (overwrites parameters!)
       * @param dx translation in x
       * @param dy translation in y
       * @param dz translation in z
       * @param alpha rotation around x
       * @param beta rotation around y
       * @param gamma rotation around z
       * @param t0 time zero
       */
      void setParameters(double dx, double dy, double dz,
                         double alpha, double beta, double gamma,
                         double t0)
      {
        m_par[0] = dx;
        m_par[1] = dy;
        m_par[2] = dz;
        m_par[3] = alpha;
        m_par[4] = beta;
        m_par[5] = gamma;
        m_par[6] = t0;
      }

      /**
       * Run a single iteration
       * @param track track parameters
       * @param hypothesis particle hypothesis
       * @return error status (0 = OK, < 0 no track hit, > 0 matrix not pos. definite)
       */
      int iterate(const TOPtrack& track, const Const::ChargedStable& hypothesis);

      /**
       * Returns module ID
       * @return module ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns alignment parameters.
       * Order is: translations in x, y, z, rotation angles around x, y, z, time zero
       * @return parameters
       */
      const std::vector<float>& getParameters() const {return m_par;}

      /**
       * Returns error matrix of alignment parameters
       * @return error matrix (7 x 7 symmetric matrix as a std::vector of 49 components)
       */
      const std::vector<float>& getErrorMatrix() const {return m_COV;}

      /**
       * Returns number of tracks used
       * @return number of tracks
       */
      int getNumTracks() const {return m_numTracks;}

      /**
       * Checks if the results are valid
       * @return true if results valid
       */
      bool isValid() const {return m_valid;}

    private:

      int m_moduleID = 0; /**< module ID */

      std::vector<float> m_par;  /**< parameters */
      std::vector<float> m_step; /**< step sizes */
      std::vector<float> m_COV;  /**< covariance matrix */
      int m_numTracks = 0;  /**< number of tracks used */
      bool m_valid = false; /**< validity of results */

      std::vector<double> m_U;   /**< matrix (neg. sum of second derivatives) */

    };


  } // TOP namespace
} // Belle2 namespace

