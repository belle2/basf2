#include <framework/pcore/zmq/processModules/ZMQRxModule.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/core/RandomNumbers.h>

#include <TSystem.h>

using namespace std;
using namespace Belle2;

ZMQRxModule::ZMQRxModule() :
  ZMQModule()
{
  setDescription("TODO");
}