#include <framework/pcore/zeromq/ZeroMQTxModule.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/core/RandomNumbers.h>

#include <TSystem.h>

using namespace std;
using namespace Belle2;

ZeroMQTxModule::ZeroMQTxModule() :
  ZeroMQModule()
{
  setDescription("TODO");
}

void ZeroMQTxModule::initialize()
{
  if (m_param_sendRandomState) {
    m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);
  }
}

void ZeroMQTxModule::setRandomState()
{
  if (m_param_sendRandomState) {
    //Save event level random generator into datastore to send it to other processes
    if (!m_randomgenerator.isValid()) {
      m_randomgenerator.construct(RandomNumbers::getEventRandomGenerator());
    } else {
      *m_randomgenerator = RandomNumbers::getEventRandomGenerator();
    }
  }
}
