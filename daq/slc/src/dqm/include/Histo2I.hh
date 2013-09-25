#ifndef _B2DQM_Histo2I_hh
#define _B2DQM_Histo2I_hh

#include "Histo2.hh"
#include "IntArray.hh"

namespace B2DQM {

  class Histo2I : public Histo2 {

  public:
    Histo2I();
    Histo2I(const Histo2I& h);
    Histo2I(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax,
            int nbiny, double ymin, double ymax);
    virtual ~Histo2I() throw() {}

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
