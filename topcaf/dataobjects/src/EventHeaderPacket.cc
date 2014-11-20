#include <topcaf/dataobjects/EventHeaderPacket.h>

using namespace Belle2;

ClassImp(EventHeaderPacket)

EventHeaderPacket::EventHeaderPacket(): Packet() {}

EventHeaderPacket::EventHeaderPacket(const unsigned int* temp_buffer,
                                     int nwords): Packet(temp_buffer, nwords)
{
  m_freeze_date = m_packet_payload[2];
  m_evt_num = m_packet_payload[3];
  m_trigger = m_packet_payload[4];
  m_flag = m_packet_payload[5];
  m_nwaves = m_packet_payload[6];
  m_aux = m_packet_payload[7];
  m_ftsw = -999999.;
}
