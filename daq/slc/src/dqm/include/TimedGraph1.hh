#ifndef _B2DQM_TimedGraph_hh
#define _B2DQM_TimedGraph_hh

#include "Histo.hh"
#include "LongArray.hh"

namespace B2DQM {

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
    void readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    void readUpdate(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    void writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    void writeUpdate(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    int getIter() const throw() { return _iter; }
    std::string toString() const throw();

  };

};

#endif

