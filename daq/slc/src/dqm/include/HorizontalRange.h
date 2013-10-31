#ifndef _Belle2_HorizontalRange_hh
#define _Belle2_HorizontalRange_hh

#include "Range.h"

namespace Belle2 {

  class HorizontalRange : public Range {

  public:

    HorizontalRange();
    HorizontalRange(const std::string& name);
    HorizontalRange(const std::string& name, double min, double max);
    ~HorizontalRange() throw();

  public:
    std::string getDataType() const throw();

  };

};

#endif

