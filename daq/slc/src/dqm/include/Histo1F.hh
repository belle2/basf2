#ifndef _B2DQM_Histo1F_hh
#define _B2DQM_Histo1F_hh

#include "Histo1.hh"
#include "FloatArray.hh"

namespace B2DQM {

  class Histo1F : public Histo1 {
    
  public:
    Histo1F() throw();
    Histo1F(const Histo1F& h) throw();
    Histo1F(const std::string& name, const std::string& title,
	    int nbinx, double min, double max) throw();
    virtual ~Histo1F() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
