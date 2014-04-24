#ifndef _Belle2_Axis_hh
#define _Belle2_Axis_hh

#include "daq/slc/base/Serializable.h"
#include "daq/slc/base/Writer.h"
#include "daq/slc/base/Reader.h"

namespace Belle2 {

  class Axis : public Belle2::Serializable {

  public:
    Axis();
    Axis(int nbin, double min, double max, const std::string& title);
    Axis(int nbin, double min, double max);
    Axis(const Axis& axis);
    virtual ~Axis() throw();

  public:
    void set(int nbin, double min, double max);
    void set(int nbin, double min, double max, const std::string& title);

    int getNbins() const throw() { return _nbins; }
    double getMax() const throw() { return _max; }
    double getMin() const throw() { return _min; }
    bool isFixedMax() const throw() { return _fixed_max; }
    bool isFixedMin() const throw() { return _fixed_min; }
    const std::string& getTitle() const throw() { return _title; }

    void setNbins(int nbins) throw() { _nbins = nbins; }
    void setMax(double max) throw() { _max = max; }
    void setMin(double min) throw() { _min = min; }
    void fixMax(bool fixed) throw() { _fixed_max = fixed; }
    void fixMin(bool fixed) throw() { _fixed_min = fixed; }
    void setTitle(const std::string& title) { _title = title; }

    virtual void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);

  public:
    const Axis& operator=(const Axis& a);

  protected:
    int _nbins;
    double _min;
    double _max;
    std::string _title;
    bool _fixed_min;
    bool _fixed_max;

  };

};

#endif
