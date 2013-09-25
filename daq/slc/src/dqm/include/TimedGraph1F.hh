#ifndef _B2DQM_TimedGraph1F_hh
#define _B2DQM_TimedGraph1F_hh

#include "TimedGraph1.hh"

namespace B2DQM {

  class TimedGraph1F : public TimedGraph1 {

  public:
    TimedGraph1F() throw();
    TimedGraph1F(const TimedGraph1F& h) throw();
    TimedGraph1F(const std::string& name,
		const std::string& title, 
		int nbinx, double xmin, double xmax) throw();
    virtual ~TimedGraph1F() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
