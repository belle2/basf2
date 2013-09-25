#ifndef _B2DQM_Histo1C_hh
#define _B2DQM_Histo1C_hh

#include "Histo1.hh"
#include "CharArray.hh"

namespace B2DQM {

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
