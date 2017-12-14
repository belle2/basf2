#ifndef _Belle2_TabbedPanel_h
#define _Belle2_TabbedPanel_h

#include "daq/slc/dqm/Panel.h"

namespace Belle2 {

  class TabbedPanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;

  public:
    TabbedPanel() throw();
    TabbedPanel(const std::string& name) throw();
    virtual ~TabbedPanel() throw();

  public:
    void add(const std::string& title, Panel* panel) throw();
    virtual void add(Panel* panel) throw();
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  private:
    std::vector<std::string> m_sub_title_v;

  };

};

#endif
