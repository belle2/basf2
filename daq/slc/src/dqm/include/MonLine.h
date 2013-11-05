#ifndef _Belle2_MonLine_hh
#define _Belle2_MonLine_hh

#include "dqm/MonShape.h"
#include "dqm/DoubleArray.h"

namespace Belle2 {

  class MonLine : public MonShape {

  public:
    MonLine() {}
    MonLine(const std::string& name) : MonShape(name) {}
    MonLine(const std::string& name, double x1, double y1, double x2, double y2);
    virtual ~MonLine() throw() {}

  public:
    void set(double x1, double y1, double x2, double y2) throw() {
      _x1 = x1; _y1 = y1; _x2 = x2; _y2 = y2;
    }

  public:
    virtual void reset() throw();
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    std::string getDataType() const throw() { return "MLN"; }

  private:
    double _x1, _y1, _x2, _y2;

  };

};

#endif

