#ifndef ECLABSPDF_H
#define ECLABSPDF_H

#include <ecl/electronId/ParameterMap.h>
#include <sstream>

namespace Belle2 {

  namespace ECL {

    class ECLAbsPdf {
    public:

      virtual double pdf(const double& eop, const double& p, const double& theta) const = 0;
      virtual void init(const char* parametersFileName) = 0;

      /* static void setEnergyUnit(const Unit:: unit = "GeV") */
      /* { */
      /*   if (unit == "GeV") { s_energy_unit = 1; } */
      /*   else if (unit == "MeV") { s_energy_unit = 1e3; } */
      /* } */

      /* static void setAngularUnit(const std::string unit = "rad") */
      /* { */
      /*   if (unit == "rad") { s_ang_unit = 1; } */
      /*   else if (unit == "deg") { s_ang_unit = 180.0 / s_Pi; } */
      /* } */

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
      static constexpr double s_sqrtPiOver2 =  1.2533141373;

      unsigned int n_theta_bins;
      unsigned int n_p_bins;
      double* theta_min;
      double* p_min;

    };

  }

}
#endif
