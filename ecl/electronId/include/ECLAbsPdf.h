#ifndef ECLABSPDF_H
#define ECLABSPDF_H

#include <ecl/electronId/ParameterMap.h>
#include <sstream>

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
      unsigned int index(unsigned int ip, unsigned int ith) const
      {
        return ith * nbins + ip;
      }

      unsigned int index(double p, double costheta) const
      {
        unsigned int ip;
        if (p > 100.) ip = nbins - 1;
        else ip = (unsigned int)(p / binw);

        if (ip == 0) ip = 1;
        if (ip >= nbins) ip = nbins - 1;

        for (unsigned int i = 0; i < nCosTheta - 1; ++i)
          if (costheta > cosThetaMin[i]) return index(ip, i);
        return index(ip, nCosTheta - 1);
      }

      virtual ~ECLAbsPdf() { delete [] cosThetaMin; }

      void init(const ParameterMap& map)
      {
        nbins = map.param("nbins");
        nCosTheta = map.param("ncth");
        binw = map.param("binw");
        cosThetaMin = new double[ nCosTheta - 1 ];
        for (unsigned int i = 0; i < nCosTheta - 1; ++i) {
          std::ostringstream nm;
          nm << "costhmin" << i ;
          cosThetaMin[i] = map.param(nm.str());
        }
      }
    protected:
      static constexpr double s_sqrt2 = 1.4142135624;
      static constexpr double s_sqrtPiOver2 =  1.2533141373;

      double binw;
      unsigned int nbins;
      unsigned int nCosTheta;
      double* cosThetaMin;
    };
  }
}
#endif
