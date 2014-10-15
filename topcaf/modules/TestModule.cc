#include <topcaf/modules/TestModule.h>

using namespace Belle2;

REG_MODULE(Test)

TestModule::TestModule() : Module()
{
  setDescription("Module used to test iTop dataobjects");
}

void TestModule::initialize()
{
  m_packet_ptr.registerInDataStore();
  m_evtheader_ptr.registerInDataStore();
  m_evtwave_ptr.registerInDataStore();
}
void TestModule::event()
{
  m_packet_ptr.create();
  std::cout << "Packet Type: " << m_packet_ptr->GetPacketType() << std::endl;

  m_evtheader_ptr.create();
  std::cout << "Event Header Packet Type: " << m_evtheader_ptr->GetPacketType() << std::endl;

  m_evtwave_ptr.create();
  std::cout << "Event Waveform Packet Type: " << m_evtwave_ptr->GetPacketType() << std::endl;

  //  B2INFO("Packet Type");//:"+m_packet_ptr->GetPacketType());
}
