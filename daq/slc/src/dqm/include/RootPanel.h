#ifndef _Belle2_RootPanel_hh
#define _Belle2_RootPanel_hh

#include "dqm/Panel.h"

namespace Belle2 {

  class RootPanel : public Panel {

  public:
    static const std::string ELEMENT_TAG;

  private:

  public:
    RootPanel() throw();
    RootPanel(const std::string& name) throw();
    virtual ~RootPanel() throw();

  public:
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  };

};

#endif
