#ifndef TIMEBASEDCORRECTIONMODULE_H
#define TIMEBASEDCORRECTIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/TopConfigurations.h>
#include <string>

namespace Belle2 {
  class TimeBasedCorrectionModule : public Module {
  public:

    TimeBasedCorrectionModule();
    ~TimeBasedCorrectionModule();

    //
    void initialize();
    void beginRun();
    void event();
    void terminate();

  private:
    std::string m_correctionFilename; // name of file with correction data
    StoreObjPtr<TopConfigurations> m_topConfig;
    double m_time2tdc;
    float* m_correctionArray;
    int m_moduleNumber;
  };

}
#endif
