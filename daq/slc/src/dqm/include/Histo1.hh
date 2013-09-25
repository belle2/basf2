#ifndef _B2DQM_Histo1_hh
#define _B2DQM_Histo1_hh

#include "Histo.hh"

namespace B2DQM {

  class Histo1 : public Histo {

  public:
    Histo1() throw();
    Histo1(const Histo1& h) throw();
    Histo1(const std::string& name, const std::string& title, 
	   int nbinx, double min, double max) throw();
    virtual ~Histo1() throw() {}

  public:
    virtual int getDim() const throw() { return 1; }
    virtual void reset() throw();
    virtual void resetAxis(int nbinx, double xmin, double xmax);
    virtual double getBinContent(int nx) const throw();
    virtual double getOverFlow() const throw();
    virtual double getUnderFlow() const throw();
    virtual void setBinContent(int nx, double data) throw();
    virtual void setOverFlow(double data) throw();
    virtual void setUnderFlow(double data) throw();
    virtual double getMaximum() const throw();
    virtual double getMinimum() const throw();
    virtual void setMaximum(double data) throw();
    virtual void setMinimum(double data) throw();
    virtual void fixMaximum(double data) throw();
    virtual void fixMinimum(double data) throw();
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);

  };

};

#endif
