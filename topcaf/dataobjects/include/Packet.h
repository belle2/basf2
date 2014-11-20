#ifndef Packet_H
#define Packet_H

#include <TObject.h>
//#include <vector>

namespace Belle2 {


#define MASK_SCROD_REV          (0x00FF0000)
#define MASK_SCROD_ID           (0x0000FFFF)

////////////////////////////////////////////////////////////////////////////////////
// Base data class for the Event Packets information as defined in:               //
// http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format //
// Author: Malachi Schram (malachi.schram@pnnl.gov)                               //
////////////////////////////////////////////////////////////////////////////////////

  class Packet : public TObject {

  public:

    typedef unsigned int packet_word_t;

    Packet();

    Packet(const unsigned int* temp_buffer, int nwords);

    //--- Getters ---//
    int GetPacketType() const {return m_type;}
    int GetScrodRev() const {return m_scrod_rev;}
    int GetScrodID() const {return m_scrod_id;}

    //--- Setters ---//

    //protected:

    std::vector<packet_word_t> m_packet_payload;
    packet_word_t  m_type, m_scrod_rev, m_scrod_id;

  private:


    ClassDef(Packet, 1);

  };
}
#endif
