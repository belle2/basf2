#ifndef _B2DQM_TimedGraph1S_hh
#define _B2DQM_TimedGraph1S_hh

#include "TimedGraph1.hh"

namespace B2DQM {

  class TimedGraph1S : public TimedGraph1 {

  public:
    TimedGraph1S() throw();
    TimedGraph1S(const TimedGraph1S& h) throw();
    TimedGraph1S(const std::string& name,
		const std::string& title, 
		int nbinx, double xmin, double xmax) throw();
    virtual ~TimedGraph1S() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
