/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <vector>
#include <string>
#include <algorithm>
#include <TMatrixDSym.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Alignment of a TOP module
     */
    class ModuleAlignment {

    public:
      /**
       * Constructor
       * @param opt PDF construction option
       */
      ModuleAlignment(PDFConstructor::EPDFOption opt = PDFConstructor::c_Rough);

      /**
       * Sets module ID
       * @param moduleID module ID
       */
      void setModuleID(int moduleID) {m_moduleID = moduleID;}

      /**
       * Sets steps for numerical calculation of derivatives
       * @param position step size for translations [cm]
       * @param angle step size for rotations [radians]
       * @param time step size for module T0 [ns]
       */
      void setSteps(double position, double angle, double time);

      /**
       * Sets initial values of parameters (overwrites current parameters!)
       * Order is: translations in x, y, z, rotation angles around x, y, z, module T0
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
        B2ERROR("TOP::ModuleAlignment::fixParameter: invalid parameter name '" << name << "'");
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
        B2ERROR("TOP::ModuleAlignment::unfixParameter: invalid parameter name '" << name << "'");
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
       * @return error status (0 = OK, < 0 no track hit, > 0 matrix not positive definite)
       */
      int iterate(TOPTrack& track, const Const::ChargedStable& hypothesis);

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
       * Returns alignment parameter names
       * @return parameter names
       */
      const std::vector<std::string>& getParameterNames() const {return m_parNames;}

      /**
       * Returns alignment parameters.
       * Order is: translations in x, y, z, rotation angles around x, y, z, module T0
       * @return parameters in double precision
       */
      const std::vector<double>& getParams() const {return m_par;}

      /**
       * Returns alignment parameters.
       * Order is: translations in x, y, z, rotation angles around x, y, z, module T0
       * @return parameters in single precision
       */
      std::vector<float> getParameters() const;

      /**
       * Returns errors on alignment parameters.
       * Order is: translations in x, y, z, rotation angles around x, y, z, module T0
       * @return errors in single precision
       */
      std::vector<float> getErrors() const;

      /**
       * Returns error matrix of alignment parameters
       * Order is: translations in x, y, z, rotation angles around x, y, z, module T0
       * @return error matrix
       */
      const TMatrixDSym& getErrorMatrix() const {return m_COV;}

      /**
       * Returns track counter
       * @return number of tracks
       */
      int getNumTracks() const {return m_numTracks;}

      /**
       * Returns number of tracks used in current result
       * @return number of tracks
       */
      int getNumUsedTracks() const {return m_numUsedTracks;}

      /**
       * Checks if the results are valid
       * @return true if results valid
       */
      bool isValid() const {return m_valid;}

      /**
       * Returns number of photons used for log likelihood calculation
       * @return number of photons
       */
      int getNumOfPhotons() const {return m_numPhotons;}

    private:

      /**
       * Returns log likelihood for given parameters
       * @param par parameters
       * @param ok status [out]
       * @return log likelihood
       */
      double getLogL(const std::vector<double>& par, bool& ok);

      /**
       * Calculates numerically first and second derivatives of log likelihood against the parameters
       * @param first a vector of first derivatives [out]
       * @param second a matrix of second derivatives [out]
       * @return status (true on success)
       */
      bool derivatives(std::vector<double>& first, TMatrixDSym& second);

      /**
       * Inverts matrix m_U using Cholesky decomposition.
       * @return error status (0 = OK, > 0 matrix not pos. definite)
       */
      int invertMatrixU();

      int m_moduleID = 0; /**< module ID */
      PDFConstructor::EPDFOption m_opt = PDFConstructor::c_Rough; /**< PDF option */

      std::vector<std::string> m_parNames; /**< parameter names */
      std::vector<double> m_parInit;  /**< initial parameter values */
      std::vector<double> m_par;  /**< current parameter values */
      std::vector<double> m_steps; /**< step sizes */
      std::vector<double> m_maxDpar; /**< maximal parameter changes in one iteration */
      std::vector<bool> m_fixed; /**< true if parameter is fixed */
      TMatrixDSym m_COV;  /**< covariance matrix */
      int m_numTracks = 0;  /**< track counter */
      int m_numUsedTracks = 0;  /**< number of tracks used */
      int m_numPhotons = 0; /**< number of photons used for log likelihood calculation */
      bool m_valid = false; /**< validity of results */

      TMatrixDSym m_U;   /**< matrix (neg. sum of second derivatives) */
      TOPTrack* m_track = 0; /**< track parameters at TOP */
      Const::ChargedStable m_hypothesis = Const::muon; /**< particle hypothesis */

    };

  } // namespace TOP
} // namespace Belle2
