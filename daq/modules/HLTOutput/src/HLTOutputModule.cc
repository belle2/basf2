#include <daq/modules/HLTOutput/HLTOutputModule.h>

using namespace Belle2;

REG_MODULE(HLTOutput)

HLTOutputModule::HLTOutputModule() : Module()
{
  setDescription("HLTOutput module");
  setPropertyFlags(c_Input);

  addParam("times", m_times, "time", 1);
}

HLTOutputModule::~HLTOutputModule()
{
  //delete m_outBuffer;
}

void HLTOutputModule::initialize()
{
  B2INFO("Module HLTOutput initializing...");
  m_buffer = new RingBuffer(c_DataOutPort);
}

void HLTOutputModule::beginRun()
{
  B2INFO("Module HLTOutput starts a run");
}

void HLTOutputModule::event()
{
  B2INFO("Module HLTOutput starts an event");
  test(m_times);
}

void HLTOutputModule::endRun()
{
  B2INFO("Module HLTOutput ends a run");
}

void HLTOutputModule::terminate()
{
  sendTerminate();
  B2INFO("Module HLTOutput terminating...");
}

void HLTOutputModule::test(unsigned int times)
{
  for (unsigned int i = 0; i < times; i++) {
    std::string testMessage("Hello, this is a new test of HLT");
    std::stringstream ss;
    ss << testMessage << i;

    B2INFO("[HLTOutput] Put a data " << ss.str() << " (" << ss.str().size()
           << " bytes) into the ring buffer");
    m_buffer->insq((int*)ss.str().c_str(), ss.str().size() / 4 + 1);
  }
}

void HLTOutputModule::sendTerminate()
{
  std::string message("Terminate");

  B2INFO("[HLTOutput] Termination code sending");
  m_buffer->insq((int*)message.c_str(), message.size() / 4 + 1);
}
