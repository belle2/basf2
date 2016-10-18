#include <framework/pcore/zeromq/ZeroMQRxModule.h>

#include <framework/pcore/EvtMessage.h>
#include <framework/core/RandomNumbers.h>

#include <TSystem.h>

using namespace std;
using namespace Belle2;

ZeroMQRxModule::ZeroMQRxModule() :
  ZeroMQModule()
{
  setDescription("TODO");
}