/*
 * Thomas Keck 2014
 */

#pragma once

#include <vector>

namespace Belle2 {

  class MockPluginInspector {

  public:
    static MockPluginInspector& GetInstance();

    void SetMvaValue(double _mva_value) { mva_value = _mva_value; }

    std::vector< std::vector<float> > GetTrainEvents() { return train_events; }
    std::vector<float> GetTrainEvent(unsigned int iEvent) { return train_events[iEvent]; }
    std::vector<float> GetTestEvent() { return test_event; }

  private:
    MockPluginInspector(const MockPluginInspector&);
    MockPluginInspector() : mva_value(0) { }

  public:
    float mva_value;
    std::vector<float> test_event;
    std::vector< std::vector<float> > train_events;

  };

}


