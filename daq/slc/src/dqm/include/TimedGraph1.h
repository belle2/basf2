#ifndef _Belle2_TimedGraph_hh
#define _Belle2_TimedGraph_hh

#include "Histo.h"
#include "LongArray.h"

namespace Belle2 {

  class TimedGraph1 : public Histo {

  protected:
    int _iter;
    LongArray _time_v;

  public :
    TimedGraph1();
    TimedGraph1(const std::string& name,
                const std::string& title,
                int nbinx, double xmin, double xmax);
    virtual ~TimedGraph1() throw();

  public:
    void reset() throw();
    int getDim() const throw() { return 1; }
    double getMaximum() const throw() { return _axis_y.getMax(); }
    double getMinimum() const throw() { return _axis_y.getMin(); }
    void setMaximum(double data) throw();
    void setMinimum(double data) throw();
    void fixMaximum(double data) throw();
    void fixMinimum(double data) throw();
    double getPoint(int n) const throw();
    void setPoint(int n, double data) throw();
    long getTime(int n) const throw();
    void setTime(int n, long long time) throw();
    void addPoint(long long time, double data) throw();
    void addPoint(double data) throw();
    double getLatestPoint() const throw();
    long getLatestTime() const throw();
    void readObject(Belle2::Reader& reader) throw(Belle2::IOException);
    void readUpdate(Belle2::Reader& reader) throw(Belle2::IOException);
    void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    void readContents(Belle2::Reader& reader) throw(Belle2::IOException);
    void writeObject(Belle2::Writer& writer) const throw(Belle2::IOException);
    void writeUpdate(Belle2::Writer& writer) const throw(Belle2::IOException);
    void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    int getIter() const throw() { return _iter; }
    std::string toString() const throw();

  };

};

#endif

