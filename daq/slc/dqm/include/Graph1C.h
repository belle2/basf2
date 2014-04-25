#ifndef _Belle2_Graph1C_h
#define _Belle2_Graph1C_h

#include "daq/slc/dqm/Graph1.h"
#include "daq/slc/dqm/CharArray.h"

namespace Belle2 {

  class Graph1C : public Graph1 {

  private:

  public:
    Graph1C();
    Graph1C(const Graph1C& h);
    Graph1C(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax, double ymin, double ymax);
    virtual ~Graph1C() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
