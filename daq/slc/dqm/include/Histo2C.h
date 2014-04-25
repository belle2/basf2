#ifndef _Belle2_Histo2C_h
#define _Belle2_Histo2C_h

#include "daq/slc/dqm/Histo2.h"
#include "daq/slc/dqm/CharArray.h"

namespace Belle2 {

  class Histo2C : public Histo2 {

  public:
    Histo2C();
    Histo2C(const Histo2C& h);
    Histo2C(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax,
            int nbiny, double ymin, double ymax);
    virtual ~Histo2C() throw() {}

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
