#ifndef ECLABSPDF_H
#define ECLABSPDF_H

#include <ecl/electronId/ParameterMap.h>
#include <sstream>
#include <TMath.h>

namespace Belle2 {

  namespace ECL {

    class ECLAbsPdf {
    public:
      virtual double pdf(double eop, double p, double costheta) const = 0;
      virtual void init(const char* parametersFileName) = 0;
      std::string name(const char* base, int i, int j) const
      {
        std::ostringstream nm;
        nm << base << i << "_" << j;
        return nm.str();
      }

      /** Return global index in linearised (p,theta) matrix
      n_rows = n_theta_bins
      n_cols = n_p_bins
      */
      unsigned int index(const unsigned int& irow_p, const unsigned int& icol_th) const
      {
        return irow_p * n_theta_bins + icol_th;
      }

      /** Return global index in linearised (p,theta) matrix
      for this p,theta bin
      */
      unsigned int index(const double& p, const double& theta) const
      {

        unsigned int ip(n_p_bins - 1);
        for (unsigned int i(ip); i >= 0; --i) {
          if (p > p_min[i]) {
            ip = i;
            break;
          }
        }
        unsigned int ith(n_theta_bins - 1);
        for (unsigned int i(ith); i >= 0; --i) {
          if (TMath::Abs(theta) > theta_min[i]) {
            ith = i;
            break;
          }
        }

        return index(ip, ith);

      }

      void init(const ParameterMap& map)
      {

        n_theta_bins = map.param("n_theta_bins");
        n_p_bins     = map.param("n_p_bins");

        theta_min = new double[n_theta_bins - 1];
        p_min     = new double[n_p_bins - 1];

        for (unsigned int i(0); i < n_theta_bins - 1; ++i) {
          std::ostringstream nm;
          nm << "theta_" << i << "_min";
          theta_min[i] = map.param(nm.str());
        }
        for (unsigned int i(0); i < n_p_bins - 1; ++i) {
          std::ostringstream nm;
          nm << "p_" << i << "_min";
          p_min[i] = map.param(nm.str());
        }

      }

      virtual ~ECLAbsPdf()
      {
        delete [] theta_min;
        delete [] p_min;
      }

      ////////////////////////////////////

    protected:
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
