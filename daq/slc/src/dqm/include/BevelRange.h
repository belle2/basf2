#ifndef BevelRange_hh
#define BevelRange_hh

#include "Range.h"

namespace Belle2 {

  class BevelRange : public Range {

  public:
    BevelRange();
    BevelRange(const std::string& name);
    BevelRange(const std::string& name, double min, double max);
    virtual ~BevelRange() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
