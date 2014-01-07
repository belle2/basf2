#ifndef _Belle2_TemplateDQMPackage_h
#define _Belle2_TemplateDQMPackage_h

#include <daq/slc/dqm/DQMPackage.h>

#include <daq/slc/dqm/MonLabel.h>
#include <daq/slc/dqm/TimedGraph1.h>

namespace Belle2 {

  class TemplateDQMPackage : public DQMPackage {

  public:
    TemplateDQMPackage(const std::string& name,
                       const std::string& filename);
    virtual ~TemplateDQMPackage() throw() {};

  public:
    virtual void init();
    virtual void update();

  private:
    Histo* h1;
    Histo* h1_2;
    Histo* h2;
    Histo* h3;
    Histo* h3_2;
    Histo* h4;
    MonLabel* label_state;
    MonLabel* label_rate1;
    MonLabel* label_rate1_2;
    MonLabel* label_rate2;
    TimedGraph1* g1;
    TimedGraph1* g1_2;
    double _time;

  };

}

#endif
