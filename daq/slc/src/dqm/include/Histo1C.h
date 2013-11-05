#ifndef _Belle2_Histo1C_hh
#define _Belle2_Histo1C_hh

#include "dqm/Histo1.h"
#include "dqm/CharArray.h"

namespace Belle2 {

  class Histo1C : public Histo1 {

  public:
    Histo1C() throw();
    Histo1C(const Histo1C& h) throw();
    Histo1C(const std::string& name, const std::string& title,
            int nbinx, double min, double max) throw();
    virtual ~Histo1C() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
