#include <topcaf/modules/TopConfigurationModule/TopConfigurationModule.h>
#include <framework/conditions/ConditionsService.h>


#include <iostream>
#include <cstdlib>

#include <TROOT.h>
#include <TKey.h>
#include <TList.h>
#include <TFile.h>
#include <TClass.h>

using namespace Belle2;

REG_MODULE(TopConfiguration)

TopConfigurationModule::TopConfigurationModule() : Module()
{
  setDescription("This module is used to load TOP configurations and various numbering conversions.  This module only works with the conditions db.");

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
  std::string filename = (ConditionsService::GetInstance()->GetPayloadFileURL(this));
  topconfig_ptr->ReadTopConfigurations(filename);
}

void TopConfigurationModule::event()
{

}

void  TopConfigurationModule::terminate()
{

}


