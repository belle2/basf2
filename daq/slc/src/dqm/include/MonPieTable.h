#ifndef _Belle2_MonPieTable_hh
#define _Belle2_MonPieTable_hh

#include "MonShape.h"
#include "FloatArray.h"

namespace Belle2 {

  class MonPieTable : public MonShape {

  public:
    MonPieTable(std::string name) : MonShape(name) {}
    MonPieTable() : MonShape() {}
    virtual ~MonPieTable() throw() {}

  public:
    virtual void reset() throw();
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual std::string getDataType() const throw() { return "MPT"; }

  public:
    float getData(int index) const throw() { return _array.get(index); }
    void setData(int index, float data) throw() { _array.set(index, data); }

  private:
    FloatArray _array;

  };

};

#endif
