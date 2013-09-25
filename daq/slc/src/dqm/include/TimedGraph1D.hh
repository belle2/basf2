#ifndef _B2DQM_TimedGraph1D_hh
#define _B2DQM_TimedGraph1D_hh

#include "TimedGraph1.hh"

namespace B2DQM {

  class TimedGraph1D : public TimedGraph1 {

  public:
    TimedGraph1D() throw();
    TimedGraph1D(const TimedGraph1D& h) throw();
    TimedGraph1D(const std::string& name,
		const std::string& title, 
		int nbinx, double xmin, double xmax) throw();
    virtual ~TimedGraph1D() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
