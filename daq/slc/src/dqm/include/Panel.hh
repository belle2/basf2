#ifndef _B2DQM_UI_Panel_hh
#define _B2DQM_UI_Panel_hh

#include "XMLElement.hh"

#include <string>
#include <vector>

namespace B2DQM {

  class Panel : public XMLElement {
    
  protected:
    std::vector<Panel*> _sub_panel_v;
    
  public:
    Panel () {}
    Panel(const std::string& name) : XMLElement(name) {}
    virtual ~Panel() throw();
    
  public:
    virtual void add(Panel* panel) throw() {
      if ( panel != NULL ) _sub_panel_v.push_back(panel);
    }
    std::vector<Panel*>& getPanels() throw() { return _sub_panel_v; }
    const std::vector<Panel*>& getPanels() const throw() { return _sub_panel_v; }

  };

};

#endif
