#include <eutel/eudaq/TLUEvent.h>

#include <ostream>

namespace eudaq {

  EUDAQ_DEFINE_EVENT(TLUEvent, str2id("_TLU"));

  TLUEvent::TLUEvent(Deserializer& ds) :
    Event(ds)
  {
    ds.read(m_extratimes);
  }

  void TLUEvent::Print(std::ostream& os) const
  {
    Event::Print(os);
    if (m_extratimes.size() > 0) {
      os << " [" << m_extratimes.size() << " extra]";
    }
  }

  void TLUEvent::Serialize(Serializer& ser) const
  {
    Event::Serialize(ser);
    ser.write(m_extratimes);
  }
}
