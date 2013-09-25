#ifndef _B2DQM_MonLine_hh
#define _B2DQM_MonLine_hh

#include "MonShape.hh"
#include "DoubleArray.hh"

namespace B2DQM {

  class MonLine : public MonShape {

  public:
    MonLine() {}
    MonLine(const std::string& name) : MonShape(name) {}
    MonLine (const std::string& name, double x1, double y1, double x2, double y2);
    virtual ~MonLine() throw() {}

  public:
    void set(double x1, double y1, double x2, double y2) throw() {
      _x1 = x1; _y1 = y1; _x2 = x2; _y2 = y2; 
    }

  public:
    virtual void reset() throw();
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    std::string getDataType() const throw() { return "MLN"; }

  private:
    double _x1, _y1, _x2, _y2;

  };

};

#endif

