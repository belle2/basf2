#ifndef _Belle2_TimedGraph_h
#define _Belle2_TimedGraph_h

#include "daq/slc/dqm/Histo.h"
#include "daq/slc/dqm/LongArray.h"

namespace Belle2 {

  class TimedGraph1 : public Histo {

  public :
    TimedGraph1();
    TimedGraph1(const std::string& name,
                const std::string& title,
                int nbinx, double xmin, double xmax);
    virtual ~TimedGraph1() throw();

  public:
    void reset() throw();
    int getDim() const throw() { return 1; }
    double getMaximum() const throw() { return m_axis_y.getMax(); }
    double getMinimum() const throw() { return m_axis_y.getMin(); }
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
    void readObject(Reader& reader) throw(IOException);
    void readUpdate(Reader& reader) throw(IOException);
    void readConfig(Reader& reader) throw(IOException);
    void readContents(Reader& reader) throw(IOException);
    void writeObject(Writer& writer) const throw(IOException);
    void writeUpdate(Writer& writer) const throw(IOException);
    void writeConfig(Writer& writer) const throw(IOException);
    void writeContents(Writer& writer) const throw(IOException);
    int getIter() const throw() { return m_iter; }
    std::string toString() const throw();

  protected:
    int m_iter;
    LongArray m_time_v;

  };

};

#endif

