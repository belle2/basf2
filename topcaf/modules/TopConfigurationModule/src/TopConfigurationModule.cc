#include <topcaf/modules/TopConfigurationModule/TopConfigurationModule.h>
//#include <framework/conditions/ConditionsService.h>


#include <iostream>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TList.h>
#include <TFile.h>
#include <TClass.h>

using namespace Belle2;
using std::string;

REG_MODULE(TopConfiguration)

TopConfigurationModule::TopConfigurationModule() : Module()
{
  setDescription("This module is used to load TOP configurations and various numbering conversions.  This module only works with the conditions db.");
  addParam("filename", m_filename, "File name for the configuration module");
}

TopConfigurationModule::~TopConfigurationModule() {}

void TopConfigurationModule::initialize()
{
  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  topconfig_ptr.registerPersistent("", DataStore::c_Persistent);


}

void TopConfigurationModule::beginRun()
{
  StoreObjPtr<TopConfigurations> topconfig_ptr("", DataStore::c_Persistent);
  topconfig_ptr.create(true);
  // std::string filename = (ConditionsService::getInstance()->getPayloadFileURL(this));
  // std::string filename = "";
  topconfig_ptr->ReadTopConfigurations(m_filename);
}

void TopConfigurationModule::event()
{

}

void  TopConfigurationModule::terminate()
{

}
