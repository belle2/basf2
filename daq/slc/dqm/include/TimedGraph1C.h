#ifndef _Belle2_TimedGraph1C_hh
#define _Belle2_TimedGraph1C_hh

#include "daq/slc/dqm/TimedGraph1.h"

namespace Belle2 {

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
