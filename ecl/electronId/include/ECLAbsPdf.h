#ifndef ECLABSPDF_H
#define ECLABSPDF_H

#include <sstream>

namespace Belle2 {
  class ECLAbsPdf {
  public:
    virtual double pdf(double eop, double p) const = 0;
    virtual void init() = 0;
    std::string name(const char* base, int i) const {
      std::ostringstream nm;
      nm << base << i;
      return nm.str();
    }
    virtual ~ECLAbsPdf() {}

    double integral(double p) const {
      int i = int(p / 0.250);
      if (i == 0) i = 1;
      if (i > 7) i = 7;
      double result = 0;
      double x = 0;
      double dx = 0.0001;
      for (int i = 0; x < 5; ++i) {
        x = i * dx;
        result += pdf(x, p) * dx;

      }
      return result;
    }
  protected:
    static constexpr double s_sqrt2 = 1.4142135624;
    static constexpr double s_sqrtPiOver2 =  1.2533141373;


  };
}
#endif
