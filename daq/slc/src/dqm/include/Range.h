#ifndef _Belle2_Range_hh
#define _Belle2_Range_hh

#include "Histo1D.h"
#include "DoubleArray.h"

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
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void readUpdate(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeUpdate(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void readUpdateFull(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeUpdateFull(Belle2::Writer& writer) const throw(Belle2::IOException);

  };

};

#endif

