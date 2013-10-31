#ifndef _Belle2_TabbedPanel_hh
#define _Belle2_TabbedPanel_hh

#include "Panel.h"

namespace Belle2 {

  class TabbedPanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;

  private:
    std::vector<std::string> _sub_title_v;

  public:
    TabbedPanel() throw();
    TabbedPanel(const std::string& name) throw();
    virtual ~TabbedPanel() throw();

  public:
    void add(const std::string& title, Panel* panel) throw();
    virtual void add(Panel* panel) throw();
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  };

};

#endif
