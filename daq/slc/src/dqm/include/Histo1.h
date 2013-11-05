#ifndef _Belle2_Histo1_hh
#define _Belle2_Histo1_hh

#include "dqm/Histo.h"

namespace Belle2 {

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
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);

  };

};

#endif
