#ifndef _B2DQM_VerticalRange_hh
#define _B2DQM_VerticalRange_hh

#include "Range.hh"

namespace B2DQM {

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
