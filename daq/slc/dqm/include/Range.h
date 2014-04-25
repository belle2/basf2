#ifndef _Belle2_Range_h
#define _Belle2_Range_h

#include "daq/slc/dqm/Histo1D.h"
#include "daq/slc/dqm/DoubleArray.h"

namespace Belle2 {

  class Range : public Histo1D {

  public:
    Range();
    Range(const std::string& name);
    Range(const std::string& name, double min, double max);
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
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void readUpdate(Reader& reader) throw(IOException);
    virtual void writeUpdate(Writer& writer) const throw(IOException);
    virtual void readUpdateFull(Reader& reader) throw(IOException);
    virtual void writeUpdateFull(Writer& writer) const throw(IOException);

  };

};

#endif

