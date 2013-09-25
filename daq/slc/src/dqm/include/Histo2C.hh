#ifndef _B2DQM_Histo2C_hh
#define _B2DQM_Histo2C_hh

#include "Histo2.hh"
#include "CharArray.hh"

namespace B2DQM {

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
