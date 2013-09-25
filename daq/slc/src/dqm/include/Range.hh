#ifndef _B2DQM_Range_hh
#define _B2DQM_Range_hh

#include "Histo1D.hh"
#include "DoubleArray.hh"

namespace B2DQM {

  class Range : public Histo1D {

  public:
    Range();
    Range(const std::string& name);
    Range (const std::string& name, double min, double max);
    virtual ~Range() throw();

  public:
    void set(double min, double max) throw();
    void fixMaximum(double data, bool fix) throw();
    void fixMinimum(double data, bool fix) throw();
    int getDim() const throw();
    double getMaximum() const throw();
    double getMinimum() const throw();
    void setMaximum(double data) throw();
    void setMinimum(double data) throw();

  public:
    virtual void reset() throw();
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void readUpdate(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeUpdate(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void readUpdateFull(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeUpdateFull(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);

  };

};

#endif

