/**********************************************************************************
 * Data class for the Event Packet: Header information as defined in:             *
 * http://www.phys.hawaii.edu/~kurtisn/doku.php?id=itop:documentation:data_format *
 * Author: Malachi Schram (malachi.schram@pnnl.gov)                               *
 **********************************************************************************/

#ifndef EventHeaderPacket_H
#define EventHeaderPacket_H

#include <topcaf/dataobjects/Packet.h>

namespace Belle2 {
//! Define Trigger
#define MASK_TRIGGER (0x000000FF)
//! Define Flag
#define MASK_FLAG (0x000000FF)

  class EventHeaderPacket: public Packet {

  public:
    //! empty default constructor
    //! do not remove. This is necessary to put this class in the event store
    EventHeaderPacket();
    EventHeaderPacket(const packet_word_t* temp_buffer, int nwords);

    ///--- Getters ---///
    inline packet_word_t GetFreezeDate() const {return m_freeze_date;}
    //! get event number
    inline packet_word_t GetEventNumber() const {return m_evt_num;}
    //! get event trigger
    inline bool GetEventTrigger() const {return (MASK_TRIGGER & m_trigger);} //use only 1st byte
    //! get event flag
    inline packet_word_t GetEventFlag() const {return (m_flag);}
    //! get wave form packets
    inline packet_word_t GetNumWaveformPackets() const {return m_nwaves;}
    //! get Auxiliary packets
    inline packet_word_t GetNumAuxiliaryPackets() const {return m_aux;}
    //! Get FTSW
    inline double GetFTSW() {return m_ftsw;}

    ///--- Setters ---///
    inline void SetFTSW(double ftsw) {m_ftsw = ftsw;}
    //! set flag
    inline void SetFlag(packet_word_t flag) {m_flag = flag;}

  private:
    //! Freeze_date
    packet_word_t m_freeze_date;
    //! Event number
    packet_word_t m_evt_num;
    //! Trigger
    packet_word_t m_trigger;
    //! Flag
    packet_word_t m_flag;
    //! Number of waves
    packet_word_t m_nwaves;
    //! Aux
    packet_word_t m_aux;
    //! FTSW
    double m_ftsw;

    ClassDef(EventHeaderPacket, 1);
  };
}
#endif
