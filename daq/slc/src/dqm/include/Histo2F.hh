#ifndef _B2DQM_Histo2F_hh
#define _B2DQM_Histo2F_hh

#include "Histo2.hh"
#include "FloatArray.hh"

namespace B2DQM {

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
