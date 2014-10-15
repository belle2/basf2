#include <topcaf/dataobjects/inc/Packet.h>

using namespace Belle2;

ClassImp(Packet)

Packet::Packet()
{
  m_type = -1;
  m_scrod_rev = -1;
  m_scrod_id = -1;
}

Packet::Packet(const unsigned int* temp_buffer, int nwords) : TObject()
{
  for (int i = 0; i < nwords; i++)
    m_packet_payload.push_back(temp_buffer[i]);

  m_type      = m_packet_payload[0];
  m_scrod_rev = (m_packet_payload[1] & MASK_SCROD_REV) >> 16;
  m_scrod_id  = (m_packet_payload[1] & MASK_SCROD_ID);

}
