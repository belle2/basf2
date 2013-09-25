#ifndef _B2DQM_Legend_hh
#define _B2DQM_Legend_hh

#include "Histo.hh"
#include "Rect.hh"

namespace B2DQM {

  class Legend : public Rect {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Legend(const std::string& name ="", 
	 float x = 0.68, float y = 0.2, float width = 0.26, float height = 0.08) throw()
      : Rect(name, x, y, width, height) {}
    virtual ~Legend() throw() {}

  public:
    void add(Histo* obj) throw();
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  private:
    std::vector<Histo*> _object_v;

  };

};

#endif
