#ifndef _B2DQM_Histo1S_hh
#define _B2DQM_Histo1S_hh

#include "Histo1.hh"
#include "ShortArray.hh"

namespace B2DQM {

  class Histo1S : public Histo1 {
    
  public:
    Histo1S() throw();
    Histo1S(const Histo1S& h) throw();
    Histo1S(const std::string& name, const std::string& title,
	    int nbinx, double min, double max) throw();
    virtual ~Histo1S() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
