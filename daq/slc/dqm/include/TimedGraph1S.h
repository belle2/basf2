#ifndef _Belle2_TimedGraph1S_hh
#define _Belle2_TimedGraph1S_hh

#include "daq/slc/dqm/TimedGraph1.h"

namespace Belle2 {

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
