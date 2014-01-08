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
  };
}
#endif
