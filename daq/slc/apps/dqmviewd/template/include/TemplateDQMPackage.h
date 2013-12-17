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
    Histo* _h_ncpr;
    Histo* _h_nevt;
    Histo* _h_size;
    Histo* _h_size2d;
    Histo* _h_ncpr_cpy;
    Histo* _h_nevt_cpy;
    Histo* _h_size_cpy;
    MonLabel* _label_state;
    MonLabel* _label_nevt;
    MonLabel* _label_nevt_rate;
    TimedGraph1* _gr_nevt;
    double _time;

  };

}

#endif
