#ifndef _B2DQM_Graph1S_hh
#define _B2DQM_Graph1S_hh

#include "Graph1.hh"
#include "ShortArray.hh"

namespace B2DQM {

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
