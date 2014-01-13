#ifndef _Belle2_StorageEnvMonPackage_h
#define _Belle2_StorageEnvMonPackage_h

#include <daq/storage/storager_data.h>

#include <daq/slc/dqm/EnvMonitorPackage.h>
#include <daq/slc/dqm/MonLabel.h>
#include <daq/slc/dqm/TimedGraph1.h>

namespace Belle2 {

  class StorageEnvMonPackage : public EnvMonitorPackage {

  public:
    StorageEnvMonPackage(const std::string& name);
    virtual ~StorageEnvMonPackage() throw() {};

  public:
    virtual void init();
    virtual bool update();

  private:
    storager_data* m_data;
    MonLabel* m_label_expno;
    MonLabel* m_label_runno;
    MonLabel* m_label_subno;
    MonLabel* m_label_evtno;
    MonLabel* m_label_nevts;
    MonLabel* m_label_starttime;
    MonLabel* m_label_runlength;
    MonLabel* m_label_data_size;
    MonLabel* m_label_data_rate;
    MonLabel* m_label_event_rate;
    TimedGraph1* m_g_data_size;
    TimedGraph1* m_g_data_rate;
    TimedGraph1* m_g_event_rate;

  };

}

#endif
