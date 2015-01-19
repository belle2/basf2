/*
 * Thomas Keck 2014
 */

#pragma once

#include <vector>

namespace Belle2 {


  /**
   * Class inspects MockPlugin class, only used to write TMVAInterface tests
   */
  class MockPluginInspector {

  public:

    /**
     * Method
     */
    static MockPluginInspector& GetInstance();


    /**
     * Method
     */
    void SetMvaValue(double _mva_value) { mva_value = _mva_value; }


    /**
     * Method
     */
    std::vector< std::vector<float> > GetTrainEvents() { return train_events; }

    /**
     * Method
     */
    std::vector<float> GetTrainEvent(unsigned int iEvent) { return train_events[iEvent]; }

    /**
     * Method
     */
    std::vector<float> GetTestEvent() { return test_event; }

    /**
     * Weight
     */
    std::vector<float> GetTrainWeights() { return train_event_weights; }

    /**
     * Weight
     */
    float GetTrainWeight(unsigned int iEvent) { return train_event_weights[iEvent]; }

    /**
     * Weight
     */
    float GetTestWeight() { return test_event_weight; }

    /**
     * Class
     */
    std::vector<int> GetTrainClasses() { return train_event_classes; }

    /**
     * Class
     */
    int GetTrainClass(unsigned int iEvent) { return train_event_classes[iEvent]; }

    /**
     * Class
     */
    int GetTestClass() { return test_event_class; }

    void clear() {
      train_events.clear();
      train_event_weights.clear();
      train_event_classes.clear();
      test_event.clear();
      mva_value = 0;
      test_event_weight = 0;
      test_event_class = 0;
    }

  private:

    /**
     * Method
     */
    MockPluginInspector(const MockPluginInspector&);

    /**
     * Method
     */
    MockPluginInspector() : mva_value(0) { }

  public:
    float mva_value; /**< returned mva value */
    float test_event_weight; /**< passed weight of event */
    int test_event_class; /**< passed class of event */
    std::vector<float> test_event; /**< passed test event */
    std::vector<float> train_event_weights; /**< passed weight of event */
    std::vector<int> train_event_classes; /**< passed class of event */
    std::vector< std::vector<float> > train_events; /**< passed train events */

  };

}


