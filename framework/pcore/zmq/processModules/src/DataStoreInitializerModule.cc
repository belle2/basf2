#include <framework/pcore/zmq/processModules/DataStoreInitializerModule.h>

#include <framework/pcore/zmq/processModules/DataStoreInitialization.h>

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