#ifndef _B2DQM_TimedGraph1I_hh
#define _B2DQM__TimedGraph1I_hh

#include "TimedGraph1.hh"

namespace B2DQM {

  class TimedGraph1I : public TimedGraph1 {

  public:
    TimedGraph1I() throw();
    TimedGraph1I(const TimedGraph1I& h) throw();
    TimedGraph1I(const std::string& name,
		const std::string& title, 
		int nbinx, double xmin, double xmax) throw();
    virtual ~TimedGraph1I() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
