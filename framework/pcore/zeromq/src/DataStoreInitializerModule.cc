#include <framework/pcore/zeromq/DataStoreInitializerModule.h>

#include <framework/pcore/zeromq/DataStoreInitialization.h>

#include <TSystem.h>

using namespace std;
using namespace Belle2;

REG_MODULE(DataStoreInitializer);

DataStoreInitializerModule::DataStoreInitializerModule() : Module()
{
  //Set module properties
  setDescription("TODO");

  addParam("inputFileName", m_param_inputFileName, "TODO");

  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);
}

void DataStoreInitializerModule::initialize()
{
  DataStoreInitialization::initializeDataStore(m_param_inputFileName);
}