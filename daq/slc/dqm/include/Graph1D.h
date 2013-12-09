#ifndef _Belle2_Graph1D_hh
#define _Belle2_Graph1D_hh

#include "daq/slc/dqm/Graph1.h"
#include "daq/slc/dqm/DoubleArray.h"

namespace Belle2 {

  class Graph1D : public Graph1 {

  private:

  public:
    Graph1D();
    Graph1D(const Graph1D& h);
    Graph1D(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax, double ymin, double ymax);
    virtual ~Graph1D() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
