#ifndef _Belle2_Graph1I_hh
#define _Belle2_Graph1I_hh

#include "daq/slc/dqm/Graph1.h"

#include "daq/slc/dqm/IntArray.h"

namespace Belle2 {

  class Graph1I : public Graph1 {

  private:

  public:
    Graph1I();
    Graph1I(const Graph1I& h);
    Graph1I(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax, double ymin, double ymax);
    virtual ~Graph1I() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
