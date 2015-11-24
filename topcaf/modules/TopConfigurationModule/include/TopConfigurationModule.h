#ifndef TopConfigurationModule_H
#define TopConfigurationModule_H

#include <framework/core/Module.h>

#include <map>
#include <string>
#include <TH1D.h>
#include <sstream>
#include <TFile.h>
#include <TProfile.h>
#include <framework/datastore/StoreObjPtr.h>

#include <topcaf/dataobjects/TopConfigurations.h>

namespace Belle2 {
  class TopConfigurationModule : public Module {

  public:

    TopConfigurationModule();
    ~TopConfigurationModule();

    void initialize();
    void beginRun();
    void event();
    void terminate();

  private:
    std::string m_filename;


  };
}
#endif
