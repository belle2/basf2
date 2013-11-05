#ifndef _Belle2_Histo1I_hh
#define _Belle2_Histo1I_hh

#include "dqm/Histo1.h"
#include "dqm/IntArray.h"

namespace Belle2 {

  class Histo1I : public Histo1 {

  public:
    Histo1I() throw();
    Histo1I(const Histo1I& h) throw();
    Histo1I(const std::string& name, const std::string& title,
            int nbinx, double min, double max) throw();
    virtual ~Histo1I() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
