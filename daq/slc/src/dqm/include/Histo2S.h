#ifndef _Belle2_Histo2S_hh
#define _Belle2_Histo2S_hh

#include "dqm/Histo2.h"
#include "dqm/ShortArray.h"

namespace Belle2 {

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
