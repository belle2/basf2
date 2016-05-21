/**********************************************************************************
 * Base data class for the Event Packets information as defined in:               *
 * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format *
 * Author: Malachi Schram (malachi.schram@pnnl.gov)                               *
 **********************************************************************************/

#ifndef Packet_H
#define Packet_H

#include <TObject.h>

namespace Belle2 {

//! define SCROD REV
#define MASK_SCROD_REV          (0x00FF0000)
//! define SCROD ID
#define MASK_SCROD_ID           (0x0000FFFF)

  class Packet : public TObject {

  public:

    typedef unsigned int packet_word_t;

    Packet();

    Packet(const packet_word_t* temp_buffer, int nwords);

    ///--- Getters ---///
    int GetPacketType() const {return m_type;}
    //! get SCROD Rev
    int GetScrodRev() const {return m_scrod_rev;}
    //! get SCROD ID
    int GetScrodID() const {return m_scrod_id;}

    //! pay load
    std::vector<packet_word_t> m_packet_payload;
    //! type, SCROD Rev and SCROD ID
    packet_word_t  m_type, m_scrod_rev, m_scrod_id;

  private:
    ClassDef(Packet, 1);

  };
}
#endif
