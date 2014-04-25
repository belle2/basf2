#ifndef _Belle2_Panel_h
#define _Belle2_Panel_h

#include "daq/slc/dqm/MonXMLElement.h"

#include <string>
#include <vector>

namespace Belle2 {

  class Panel : public MonXMLElement {

  public:
    Panel() {}
    Panel(const std::string& name) : MonXMLElement(name) {}
    virtual ~Panel() throw();

  public:
    virtual void add(Panel* panel) throw() {
      if (panel != NULL) m_sub_panel_v.push_back(panel);
    }
    std::vector<Panel*>& getPanels() throw() { return m_sub_panel_v; }
    const std::vector<Panel*>& getPanels() const throw() { return m_sub_panel_v; }

  protected:
    std::vector<Panel*> m_sub_panel_v;

  };

};

#endif
