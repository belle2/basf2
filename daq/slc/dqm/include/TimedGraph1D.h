#ifndef _Belle2_TimedGraph1D_h
#define _Belle2_TimedGraph1D_h

#include "daq/slc/dqm/TimedGraph1.h"

namespace Belle2 {

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
