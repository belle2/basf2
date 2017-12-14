#ifndef _Belle2_HorizontalRange_h
#define _Belle2_HorizontalRange_h

#include "daq/slc/dqm/Range.h"

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

