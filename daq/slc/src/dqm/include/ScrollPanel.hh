#ifndef _B2DQM_ScrollPanel_hh
#define _B2DQM_ScrollPanel_hh

#include "Panel.hh"

namespace B2DQM {
  
  class ScrollPanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;
    
  public:
    ScrollPanel() throw();
    ScrollPanel(const std::string& name) throw();
    virtual ~ScrollPanel() throw();
    
  public:
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();
    
  };

};

#endif
