#ifndef _Belle2_Axis_h
#define _Belle2_Axis_h

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

    int getNbins() const throw() { return m_nbins; }
    double getMax() const throw() { return m_max; }
    double getMin() const throw() { return m_min; }
    bool isFixedMax() const throw() { return m_fixed_max; }
    bool isFixedMin() const throw() { return m_fixed_min; }
    const std::string& getTitle() const throw() { return m_title; }

    void setNbins(int nbins) throw() { m_nbins = nbins; }
    void setMax(double max) throw() { m_max = max; }
    void setMin(double min) throw() { m_min = min; }
    void fixMax(bool fixed) throw() { m_fixed_max = fixed; }
    void fixMin(bool fixed) throw() { m_fixed_min = fixed; }
    void setTitle(const std::string& title) { m_title = title; }

    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  public:
    const Axis& operator=(const Axis& a);

  protected:
    int m_nbins;
    double m_min;
    double m_max;
    std::string m_title;
    bool m_fixed_min;
    bool m_fixed_max;

  };

};

#endif
