#ifndef _Belle2_AbstractMonitor_h
#define _Belle2_AbstractMonitor_h

#include "dqm/HistoPackage.h"
#include "dqm/RootPanel.h"

#include "nsm/NSMData.h"

namespace Belle2 {

  class AbstractMonitor {

  public:
    AbstractMonitor(const std::string& name);
    virtual ~AbstractMonitor() throw();

  public:
    virtual void init() throw() = 0;
    virtual void update(NSMData* data) throw() = 0;
    HistoPackage* getPackage() throw() { return m_package; }
    RootPanel* getRootPanel() throw() { return m_panel; }

  private:
    HistoPackage* m_package;
    RootPanel* m_panel;

  };

}

#endif


