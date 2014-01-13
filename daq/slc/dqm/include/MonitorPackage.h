#ifndef _Belle2_MonitorPackage_h
#define _Belle2_MonitorPackage_h

#include "daq/slc/dqm/HistoPackage.h"
#include "daq/slc/dqm/RootPanel.h"

namespace Belle2 {

  class MonitorPackage {

  public:
    MonitorPackage(const std::string& name);
    virtual ~MonitorPackage() throw();

  public:
    virtual void init() = 0;
    virtual bool update() = 0;

  public:
    HistoPackage* getPackage() throw() { return m_package; }
    RootPanel* getRootPanel() throw() { return m_panel; }

  private:
    HistoPackage* m_package;
    RootPanel* m_panel;

  };

}

#endif


