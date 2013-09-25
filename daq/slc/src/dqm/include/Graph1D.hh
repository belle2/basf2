#ifndef _B2DQM_Graph1D_hh
#define _B2DQM_Graph1D_hh

#include "Graph1.hh"
#include "DoubleArray.hh"

namespace B2DQM {

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
