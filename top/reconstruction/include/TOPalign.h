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
#include <framework/logging/Logger.h>
#include <vector>
#include <string>
#include <algorithm>

namespace Belle2 {
  namespace TOP {

    /**
     * Alignment of a TOP module: core of the method is coded in fortran
     */
    class TOPalign {
    public:

      /**
       * Constructor
       */
      TOPalign();

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
       * @return data size (or 0/negative on error)
       */
      static int addData(int moduleID, int pixelID, double time, double timeError);

      /**
       * Sets expected photon yields
       * @param bkgPerModule estimation for minimal number of background hits
       * @param scaleN0 scale factor for figure-of-merit N0
       */
      static void setPhotonYields(double bkgPerModule, double scaleN0 = 1);

      /**
       * Sets module ID
       * @param moduleID module ID
       */
      void setModuleID(int moduleID) {m_moduleID = moduleID;}

      /**
       * Sets steps for numerical calculation of derivatives
       * @param position step size for translations [cm]
       * @param angle step size for rotations [radians]
       * @param time step size for T0 [ns]
       * @param refind step size for refractive index scale factor
       */
      void setSteps(double position, double angle, double time, double refind);

      /**
       * Sets grid for averaging of time-of-propagation in analytic PDF
       * @param NP number of emission points along track
       * @param NC number of Cerenkov angles
       */
      void setGrid(int NP, int NC)
      {
        m_NP = NP;
        m_NC = NC;
      }

      /**
       * Sets initial values of parameters (overwrites current parameters!)
       * Order is: translations in x, y, z, rotation angles around x, y, z, t0, dn/n
       * @param parInit initial values
       */
      void setParameters(const std::vector<double>& parInit)
      {
        for (size_t i = 0; i < std::min(parInit.size(), m_parInit.size()); i++) {
          m_parInit[i] = parInit[i];
        }
        m_par = m_parInit;
      }

      /**
       * Fixes parameter with its name given as argument
       * @param name parameter name
       */
      void fixParameter(const std::string& name)
      {
        for (unsigned i = 0; i < m_parNames.size(); i++) {
          if (name == m_parNames[i]) {
            m_fixed[i] = true;
            return;
          }
        }
        B2ERROR("TOPalign::fixParameter: invalid parameter name '" << name << "'");
      }

      /**
       * Unfixes parameter with its name given as argument
       * @param name parameter name
       */
      void unfixParameter(const std::string& name)
      {
        for (unsigned i = 0; i < m_parNames.size(); i++) {
          if (name == m_parNames[i]) {
            m_fixed[i] = false;
            return;
          }
        }
        B2ERROR("TOPalign::unfixParameter: invalid parameter name '" << name << "'");
      }

      /**
       * Unfixes all parameters
       */
      void unfixAll()
      {
        for (unsigned i = 0; i < m_fixed.size(); i++) m_fixed[i] = false;
      }

      /**
       * Run a single iteration
       * @param track track parameters
       * @param hypothesis particle hypothesis
       * @return error status (0 = OK, < 0 no track hit, > 0 matrix not pos. definite)
       */
      int iterate(const TOPtrack& track, const Const::ChargedStable& hypothesis);

      /**
       * Reset the object
       */
      void reset();

      /**
       * Returns module ID
       * @return module ID
       */
      int getModuleID() const {return m_moduleID;}

      /**
       * Returns alignment parameters.
       * Order is: translations in x, y, z, rotation angles around x, y, z, t0, dn/n
       * @return parameters
       */
      const std::vector<float>& getParameters() const {return m_par;}

      /**
       * Returns alignment parameter names
       * @return parameter names
       */
      const std::vector<std::string>& getParameterNames() const {return m_parNames;}

      /**
       * Returns errors on alignment parameters.
       * Order is: translations in x, y, z, rotation angles around x, y, z, t0, dn/n
       * @return errors
       */
      std::vector<float> getErrors() const;

      /**
       * Returns error matrix of alignment parameters
       * @return error matrix (N x N symmetric matrix as a std::vector of N*N components)
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
      int m_opt = 0; /**< PDF option (=rough) */
      int m_NP = 0;  /**< grid for averaging: number of emission points along track */
      int m_NC = 0;  /**< grid for averaging: number of Cerenkov angles */

      std::vector<std::string> m_parNames; /**< parameter names */
      std::vector<float> m_parInit;  /**< initial parameter values */
      std::vector<float> m_par;  /**< current parameter values */
      std::vector<float> m_steps; /**< step sizes */
      std::vector<float> m_maxDpar; /**< maximal parameter changes in one iteration */
      std::vector<bool> m_fixed; /**< true if parameter is fixed */
      std::vector<float> m_COV;  /**< covariance matrix */
      int m_numTracks = 0;  /**< number of tracks used */
      bool m_valid = false; /**< validity of results */

      std::vector<double> m_U;   /**< matrix (neg. sum of second derivatives) */

    };


  } // TOP namespace
} // Belle2 namespace

