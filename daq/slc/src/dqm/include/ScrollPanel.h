#ifndef _Belle2_ScrollPanel_hh
#define _Belle2_ScrollPanel_hh

#include "dqm/Panel.h"

namespace Belle2 {

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
