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
    std::vector<float> test_event; /**< passed test event */
    std::vector< std::vector<float> > train_events; /**< passed train events */

  };

}


