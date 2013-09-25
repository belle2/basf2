#ifndef _B2DQM_TimedGraph1C_hh
#define _B2DQM_TimedGraph1C_hh

#include "TimedGraph1.hh"

namespace B2DQM {

  class TimedGraph1C : public TimedGraph1 {

  public:
    TimedGraph1C() throw();
    TimedGraph1C(const TimedGraph1C& h) throw();
    TimedGraph1C(const std::string& name, const std::string& title, 
		int nbinx, double xmin, double xmax) throw();
    virtual ~TimedGraph1C() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
