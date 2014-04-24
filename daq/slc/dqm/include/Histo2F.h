#ifndef _Belle2_Histo2F_hh
#define _Belle2_Histo2F_hh

#include "daq/slc/dqm/Histo2.h"
#include "daq/slc/dqm/FloatArray.h"

namespace Belle2 {

  class Histo2F : public Histo2 {

  public:
    Histo2F();
    Histo2F(const Histo2F& h);
    Histo2F(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax,
            int nbiny, double ymin, double ymax);
    virtual ~Histo2F() throw() {}

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
