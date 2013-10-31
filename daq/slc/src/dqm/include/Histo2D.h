#ifndef _Belle2_Histo2D_hh
#define _Belle2_Histo2D_hh

#include "Histo2.h"
#include "DoubleArray.h"

namespace Belle2 {

  class Histo2D : public Histo2 {

  public:
    Histo2D();
    Histo2D(const Histo2D& h);
    Histo2D(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax,
            int nbiny, double ymin, double ymax);
    virtual ~Histo2D() throw() {}

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
