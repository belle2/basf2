#ifndef _B2DQM_Graph1_hh
#define _B2DQM_Graph1_hh

#include "Histo.hh"

namespace B2DQM {

  class Graph1 : public Histo {

  public:

    Graph1() throw();
    Graph1(const Graph1& h) throw();
    Graph1(std::string name, std::string title, 
	   int nbinx, double xmin, double xmax, 
	   double ymin, double ymax) throw();
    virtual ~Graph1() throw();

  public:

    virtual double getPointX(int n) const throw();	
    virtual double getPointY(int n) const throw();	
    virtual void setPointX(int n, double data) throw();	
    virtual void setPointY(int n, double data) throw();	
    virtual double getMaximum() const throw();
    virtual double getMinimum() const throw();
    virtual void setMaximum(double data) throw();
    virtual void setMinimum(double data) throw();
    virtual void fixMaximum(double data) throw();
    virtual void fixMinimum(double data) throw();
    virtual void reset() throw();
    virtual int getDim() const throw() { return 1; }

  public:
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);

  };

};

#endif
