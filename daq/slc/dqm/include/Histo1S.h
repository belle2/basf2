#ifndef _Belle2_Histo1S_hh
#define _Belle2_Histo1S_hh

#include "daq/slc/dqm/Histo1.h"
#include "daq/slc/dqm/ShortArray.h"

namespace Belle2 {

  class Histo1S : public Histo1 {

  public:
    Histo1S() throw();
    Histo1S(const Histo1S& h) throw();
    Histo1S(const std::string& name, const std::string& title,
            int nbinx, double min, double max) throw();
    virtual ~Histo1S() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
