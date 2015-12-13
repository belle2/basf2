#include <topcaf/dataobjects/EventHeaderPacket.h>

using namespace Belle2;

EventHeaderPacket::EventHeaderPacket()
  : Packet()
  , m_freeze_date(0)
  , m_evt_num(0)
  , m_trigger(0)
  , m_flag(0)
  , m_nwaves(0)
  , m_aux(0)
  , m_ftsw(0)
{

};


EventHeaderPacket::EventHeaderPacket(const unsigned int* temp_buffer,
                                     int nwords): Packet(temp_buffer, nwords)
{
  m_type = m_packet_payload[0];
  m_freeze_date = m_packet_payload[2];
  m_evt_num = m_packet_payload[3];
  m_trigger = m_packet_payload[4];
  m_flag = m_packet_payload[5];
  m_nwaves = m_packet_payload[6];
  m_aux = m_packet_payload[7];
  m_ftsw = -999999.;
}
