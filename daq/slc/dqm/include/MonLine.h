#ifndef _Belle2_MonLine_h
#define _Belle2_MonLine_h

#include "daq/slc/dqm/MonShape.h"
#include "daq/slc/dqm/DoubleArray.h"

namespace Belle2 {

  class MonLine : public MonShape {

  public:
    MonLine() {}
    MonLine(const std::string& name) : MonShape(name) {}
    MonLine(const std::string& name, double x1, double y1, double x2, double y2);
    virtual ~MonLine() throw() {}

  public:
    void set(double x1, double y1, double x2, double y2) throw() {
      m_x1 = x1; m_y1 = y1; m_x2 = x2; m_y2 = y2;
    }

  public:
    virtual void reset() throw();
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException);
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException);
    std::string getDataType() const throw() { return "MLN"; }

  private:
    double m_x1, m_y1, m_x2, m_y2;

  };

};

#endif

