#ifndef _B2DQM_Axis_hh
#define _B2DQM_Axis_hh

#include <util/Serializable.hh>
#include <util/Writer.hh>
#include <util/Reader.hh>

namespace B2DQM {

  class Axis : public B2DAQ::Serializable {

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
    const std::string& getTitle() const throw(){ return _title; }
    
    void setNbins(int nbins) throw() { _nbins = nbins; }
    void setMax(double max) throw() { _max = max; }
    void setMin(double min) throw() { _min = min; }
    void fixMax(bool fixed) throw() { _fixed_max = fixed; }
    void fixMin(bool fixed) throw() { _fixed_min = fixed; }
    void setTitle(const std::string& title) { _title = title; }

    virtual void readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);

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
