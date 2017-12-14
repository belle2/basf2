#ifndef _Belle2_VerticalRange_h
#define _Belle2_VerticalRange_h

#include "daq/slc/dqm/Range.h"

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
