#include <daq/modules/HLTInput/HLTInputModule.h>

using namespace Belle2;

REG_MODULE(HLTInput)

HLTInputModule::HLTInputModule() : Module()
{
  setDescription("HLTInput module");
  setPropertyFlags(c_Input);
}

HLTInputModule::~HLTInputModule()
{
}

void HLTInputModule::initialize()
{
  B2INFO("Module HLTInput initializing...");
  m_buffer = new RingBuffer(c_DataOutPort);
}

void HLTInputModule::beginRun()
{
  B2INFO("Module HLTInput starts a run");
  B2INFO("For the test");
}

void HLTInputModule::event()
{
  B2INFO("Module HLTInput starts an event");

  while (1) {
    if (m_buffer->numq() > 0) {
      char buffer[gMaxReceives];
      m_buffer->remq((int*)buffer);
      std::string given(buffer);

      B2INFO("[HLTInput] Gets a message " << given);
      if (given == "Terminate") {
        B2INFO("[HLTInput] Terminate detected!");
        break;
      }
    }
  }
}

void HLTInputModule::endRun()
{
  B2INFO("Module HLTInput ends a run");
}

void HLTInputModule::terminate()
{
  B2INFO("Module HLTInput terminating...");
}
