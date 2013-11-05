#ifndef _Belle2_Graph1S_hh
#define _Belle2_Graph1S_hh

#include "dqm/Graph1.h"
#include "dqm/ShortArray.h"

namespace Belle2 {

  class Graph1S : public Graph1 {

  private:

  public:
    Graph1S();
    Graph1S(const Graph1S& h);
    Graph1S(const std::string& name, const std::string& title,
            int nbinx, double xmin, double xmax, double ymin, double ymax);
    virtual ~Graph1S() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
