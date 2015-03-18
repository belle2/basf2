#include <tracking/trackFindingCDC/tmva/TMVA.h>
#include <TMVA/Reader.h>
#include <TMVA/Factory.h>
#include <TMVA/Tools.h>
#include <TMVA/Config.h>
#include <TPluginManager.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    namespace TMVAUtilities {
      void loadPlugins(const std::string& name)
      {
        std::string base = "TMVA@@MethodBase";
        std::string regexp1 = std::string(".*_") + name + std::string(".*");
        std::string regexp2 = std::string(".*") + name + std::string(".*");
        std::string className = std::string("TMVA::Method") + name;
        std::string pluginName = std::string("TMVA") + name;
        std::string ctor1 = std::string("Method") + name + std::string("(DataSetInfo&,TString)");
        std::string ctor2 = std::string("Method") + name + std::string("(TString&,TString&,DataSetInfo&,TString&)");

        gPluginMgr->AddHandler(base.c_str(), regexp1.c_str(), className.c_str(), pluginName.c_str(), ctor1.c_str());
        gPluginMgr->AddHandler(base.c_str(), regexp2.c_str(), className.c_str(), pluginName.c_str(), ctor2.c_str());
      }
    }
  }
}
