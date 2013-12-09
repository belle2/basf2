#ifndef _Belle2_ArichHVMonitor_h
#define _Belle2_ArichHVMonitor_h

#include <daq/slc/dqm/AbstractMonitor.h>
#include <daq/slc/dqm/MonLabel.h>
#include <daq/slc/dqm/TimedGraph1.h>

namespace Belle2 {

  class ArichHVMonitor : public AbstractMonitor {

  public:
    ArichHVMonitor();
    virtual ~ArichHVMonitor() throw() {};

  public:
    virtual void init() throw();
    virtual void update(NSMData* data) throw();

  private:
    MonLabel* _label_state;
    MonLabel* _label_voltage_mon;
    MonLabel* _label_current_mon;
    MonLabel* _label_voltage_demand;
    MonLabel* _label_voltage_limit;
    MonLabel* _label_current_limit;
    TimedGraph1* _gr_voltage;
    TimedGraph1* _gr_current;
    double _time;


  };

}

#endif

