#ifndef _Belle2_Histo1D_hh
#define _Belle2_Histo1D_hh

#include "dqm/Histo1.h"
#include "dqm/DoubleArray.h"

namespace Belle2 {

  class Histo1D : public Histo1 {

  public:
    Histo1D() throw();
    Histo1D(const Histo1D& h) throw();
    Histo1D(const std::string& name, const std::string& title,
            int nbinx, double min, double max) throw();
    virtual ~Histo1D() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
