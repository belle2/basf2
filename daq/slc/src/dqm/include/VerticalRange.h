#ifndef _Belle2_VerticalRange_hh
#define _Belle2_VerticalRange_hh

#include "Range.h"

namespace Belle2 {

  class VerticalRange : public Range {

  public:
    VerticalRange();
    VerticalRange(const std::string& name);
    VerticalRange(const std::string& name, double min, double max);
    virtual ~VerticalRange() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
