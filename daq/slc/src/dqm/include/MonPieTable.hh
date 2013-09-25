#ifndef _B2DQM_MonPieTable_hh
#define _B2DQM_MonPieTable_hh

#include "MonShape.hh"
#include "FloatArray.hh"

namespace B2DQM {

  class MonPieTable : public MonShape {
    
  public:
    MonPieTable(std::string name) : MonShape(name) {}
    MonPieTable() : MonShape() {}
    virtual ~MonPieTable() throw() {}
    
  public:
    virtual void reset() throw();
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual std::string getDataType() const throw() { return "MPT"; }

  public: 
    float getData(int index) const throw() { return _array.get(index); }
    void setData(int index, float data) throw() { _array.set(index, data); }
    
  private:
    FloatArray _array;
    
  };

};

#endif
