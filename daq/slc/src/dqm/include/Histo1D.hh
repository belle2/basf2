#ifndef _B2DQM_Histo1D_hh
#define _B2DQM_Histo1D_hh

#include "Histo1.hh"
#include "DoubleArray.hh"

namespace B2DQM {

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
