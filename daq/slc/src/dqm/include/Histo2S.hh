#ifndef _B2DQM_Histo2S_hh
#define _B2DQM_Histo2S_hh

#include "Histo2.hh"
#include "ShortArray.hh"

namespace B2DQM {

  class Histo2S : public Histo2 {

  public:
    Histo2S();
    Histo2S(const Histo2S& h);
    Histo2S(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax,
            int nbiny, double ymin, double ymax);
    virtual ~Histo2S() throw() {}

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
