#ifndef _Belle2_ScrollPanel_h
#define _Belle2_ScrollPanel_h

#include "daq/slc/dqm/Panel.h"

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
