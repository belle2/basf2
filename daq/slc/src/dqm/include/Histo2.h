#ifndef _Belle2_Histo2_hh
#define _Belle2_Histo2_hh

#include "dqm/Histo.h"
#include "dqm/CharArray.h"

namespace Belle2 {

  class Histo2 : public Histo {

  public:
    Histo2() throw();
    Histo2(const Histo2& h) throw();
    Histo2(const std::string& name, const std::string& title,
           int nbinx, double xmin, double xmax,
           int nbiny, double ymin, double ymax) throw();
    virtual ~Histo2() throw() {}

  public:
    virtual int getDim() const throw() { return 2; }
    virtual void reset() throw();
    virtual void resetAxis(int nbinx, double xmin, double xmax,
                           int nbiny, double ymin, double ymax);
    virtual double getBinContent(int nx, int ny) const throw();
    virtual double getOverFlow() const throw();
    virtual double getUnderFlow() const throw();
    virtual double getOverFlowX(int ny) const throw();
    virtual double getUnderFlowX(int ny) const throw();
    virtual double getOverFlowY(int nx) const throw();
    virtual double getUnderFlowY(int nx) const throw();
    virtual void setBinContent(int nx, int ny, double data) throw();
    virtual void setOverFlow(double data) throw();
    virtual void setOverFlowX(int ny, double data) throw();
    virtual void setOverFlowY(int nx, double data) throw();
    virtual void setUnderFlow(double data) throw();
    virtual void setUnderFlowX(int ny, double data) throw();
    virtual void setUnderFlowY(int nx, double data) throw();
    virtual double getMaximum() const throw();
    virtual double getMinimum() const throw();
    virtual void setMaximum(double data) throw();
    virtual void setMinimum(double data) throw();
    virtual void fixMaximum(double data) throw();
    virtual void fixMinimum(double data) throw();
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);

  };

}

#endif
