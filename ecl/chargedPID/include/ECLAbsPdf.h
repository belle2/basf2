/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Base class for ECL E/p PDFs                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Marco Milesi (marco.milesi@unimelb.edu.au)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <ecl/chargedPID/ParameterMap.h>
#include <sstream>

namespace Belle2 {

  namespace ECL {

    struct Parameters;

    /** Base abstract class describing the PDF (E/p) in the ECL for a generic charged particle.
    Derived classes are defined for each particle hypothesis.
     */
    class ECLAbsPdf {
    public:

      /** Reconstructs the PDF analytical forms from the configuration parameters.
      @param parametersFileName the name of the '.dat' file for a given charged particle.
       */
      virtual void init(const char* parametersFileName) = 0;

      /** Ensure the energy unit is set to be consistent w/ the one used in the configuration file.
       */
      static void setEnergyUnit(const double& unit) { s_energy_unit = unit; }

      /** Ensure the angular unit is set to be consistent w/ the one used in the configuration file.
       */
      static void setAngularUnit(const double& unit) { s_ang_unit = unit; }

      /** Getter for the energy unit that is being used.
       */
      inline double getEnergyUnit() { return s_energy_unit; }

      /** Getter for the angular unit that is being used.
       */
      inline double getAngularUnit() { return s_ang_unit; }

      /** Reconstructs the parameter name in the configuration file for a given momentum and polar angle.
      @param base the base string identifying the parameter
      @ i the index of the P bin
      @ j the index of the theta bin
       */
      std::string name(const char* base, int i, int j) const
      {
        std::ostringstream nm;
        nm << base << i << "_" << j;
        return nm.str();
      }

      /** Returns global index in linearised (p,theta) matrix.
          @param i_row_p the index of the P bin
          @param i_col_th the index of the theta bin
      */
      inline unsigned int index(unsigned int irow_p, unsigned int icol_th) const
      {
        return irow_p * n_theta_bins + icol_th;
      }

      /** Returns global index in linearised (p,theta) matrix based on particle's properties.
      @ p the particle momentum
      @ theta the particle polar angle
      */
      unsigned int index(const double& p, const double& theta) const;

      /** Stores the lower edge values of the p and theta bins, reading them form the configuration file.
      @param map a map object w/ the content of the configuration file
       */
      void init(const ParameterMap& map);

      /** Accessor of the pdf given E/p, and the particle's momentum and theta
       */
      double pdf(const double& eop, const double& p, const double& theta) const
      {
        return pdffunc(eop, index(p, theta));
      };
      /** Accessor of the pdf given E/p, and the particle's momentum and theta bin indexes
       */
      double pdf(const double& eop, unsigned int idx_p, unsigned int idx_th) const
      {
        return pdffunc(eop, index(idx_p, idx_th));
      };

      /** Destructor */
      virtual ~ECLAbsPdf()
      {
        delete [] theta_min;
        delete [] p_min;
      }

    protected:

      /** The energy unit used in the .dat file
       */
      static double s_energy_unit;
      /** The energy unit used in the .dat file
       */
      static double s_ang_unit;

      static constexpr double s_Pi = 3.14159265359;
      static constexpr double s_sqrt2 = 1.4142135624;
      static constexpr double s_sqrtPiOver2 =  1.2533141373;

      /** The number of theta bins in the .dat file
       */
      unsigned int n_theta_bins;

      /** The number of p bins in the .dat file
       */
      unsigned int n_p_bins;

      /** Array to store the lower edges of the theta bins from the .dat file
       */
      double* theta_min;

      /** Array to store the lower edges of the p bins from the .dat file
       */
      double* p_min;

      /** Getter for the pdf value of a given particle. This method is implemented
      differently by each of the children classes.
      @param eop the E/p of the particle
      @param i the global index corresponding to the particle's (p,theta) bin
      @return the value of the pdf stored in an instance of this class.
       */
      virtual double pdffunc(const double& eop, unsigned int i) const = 0;

    };

  }

}
