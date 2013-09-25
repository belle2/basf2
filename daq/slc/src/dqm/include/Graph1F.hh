#ifndef _B2DQM_Graph1F_hh
#define _B2DQM_Graph1F_hh

#include "Graph1.hh"
#include "FloatArray.hh"

namespace B2DQM {

  class Graph1F : public Graph1 {

  private:

  public:
    Graph1F();
    Graph1F(const Graph1F& h);
    Graph1F(const std::string& name, const std::string& title, 
	    int nbinx, double xmin, double xmax, double ymin, double ymax);
    virtual ~Graph1F() throw();

  public:
    virtual std::string getDataType() const throw();

  };

};

#endif
