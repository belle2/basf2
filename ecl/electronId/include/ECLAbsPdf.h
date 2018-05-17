/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Base class for ECL E/p PDFs                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <ecl/electronId/ParameterMap.h>
#include <sstream>

namespace Belle2 {

  namespace ECL {

    class ECLAbsPdf {
    public:

      virtual double pdf(const double& eop, const double& p, const double& theta) const = 0;
      virtual void init(const char* parametersFileName) = 0;

      static void setEnergyUnit(const double& unit) { s_energy_unit = unit; }
      static void setAngularUnit(const double& unit) { s_ang_unit = unit; }

      inline double getEnergyUnit() { return s_energy_unit; }
      inline double getAngularUnit() { return s_ang_unit; }

      std::string name(const char* base, int i, int j) const
      {
        std::ostringstream nm;
        nm << base << i << "_" << j;
        return nm.str();
      }

      /** Returns global index in linearised (p,theta) matrix
          n_rows = n_theta_bins
          n_cols = n_p_bins
      */
      inline unsigned int index(const unsigned int& irow_p, const unsigned int& icol_th) const
      {
        return irow_p * n_theta_bins + icol_th;
      }

      /** Returns global index in linearised (p,theta) matrix
          for this p,theta bin
      */
      unsigned int index(const double& p, const double& theta) const;

      void init(const ParameterMap& map);

      virtual ~ECLAbsPdf()
      {
        delete [] theta_min;
        delete [] p_min;
      }

      ////////////////////////////////////

    protected:

      /** The energy unit in the .dat files
      */
      static double s_energy_unit;
      static double s_ang_unit;

      static constexpr double s_Pi = 3.14159265359;
      static constexpr double s_sqrt2 = 1.4142135624;
      static constexpr double s_sqrtPiOver2 = 1.2533141373;

      unsigned int n_theta_bins;
      unsigned int n_p_bins;
      double* theta_min;
      double* p_min;

    };

  }

}

