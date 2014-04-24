#ifndef _Belle2_Panel_hh
#define _Belle2_Panel_hh

#include "daq/slc/dqm/MonXMLElement.h"

#include <string>
#include <vector>

namespace Belle2 {

  class Panel : public MonXMLElement {

  protected:
    std::vector<Panel*> _sub_panel_v;

  public:
    Panel() {}
    Panel(const std::string& name) : MonXMLElement(name) {}
    virtual ~Panel() throw();

  public:
    virtual void add(Panel* panel) throw() {
      if (panel != NULL) _sub_panel_v.push_back(panel);
    }
    std::vector<Panel*>& getPanels() throw() { return _sub_panel_v; }
    const std::vector<Panel*>& getPanels() const throw() { return _sub_panel_v; }

  };

};

#endif
