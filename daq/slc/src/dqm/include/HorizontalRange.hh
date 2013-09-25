#ifndef _B2DQM_HorizontalRange_hh
#define _B2DQM_HorizontalRange_hh

#include "Range.hh"

namespace B2DQM {

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

