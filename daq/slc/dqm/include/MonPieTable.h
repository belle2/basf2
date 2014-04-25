#ifndef _Belle2_MonPieTable_h
#define _Belle2_MonPieTable_h

#include "daq/slc/dqm/MonShape.h"
#include "daq/slc/dqm/FloatArray.h"

namespace Belle2 {

  class MonPieTable : public MonShape {

  public:
    MonPieTable(std::string name) : MonShape(name) {}
    MonPieTable() : MonShape() {}
    virtual ~MonPieTable() throw() {}

  public:
    virtual void reset() throw();
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException);
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException);
    virtual std::string getDataType() const throw() { return "MPT"; }

  public:
    float getData(int index) const throw() { return m_array.get(index); }
    void setData(int index, float data) throw() { m_array.set(index, data); }

  private:
    FloatArray m_array;

  };

};

#endif
