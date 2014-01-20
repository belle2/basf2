#include <eutel/eudaq/DetectorEvent.h>
#include <eutel/eudaq/RawDataEvent.h>

#include <ostream>

namespace eudaq {

  EUDAQ_DEFINE_EVENT(DetectorEvent, str2id("_DET"));

  DetectorEvent::DetectorEvent(Deserializer& ds) :
    Event(ds)
  {
    unsigned n;
    ds.read(n);
    //std::cout << "Num=" << n << std::endl;
    for (size_t i = 0; i < n; ++i) {
      counted_ptr<Event> ev(EventFactory::Create(ds));
      m_events.push_back(ev);
    }
  }

  void DetectorEvent::AddEvent(counted_ptr<Event> evt)
  {
    if (!evt.get()) EUDAQ_THROW("Adding null event!");
    m_events.push_back(evt);
    SetFlags(evt->GetFlags());
  }

  void DetectorEvent::Print(std::ostream& os) const
  {
    Event::Print(os);
    os << " {\n";
    for (size_t i = 0; i < NumEvents(); ++i) {
      os << "  " << *GetEvent(i) << std::endl;
    }
    os << "}";
  }

  void DetectorEvent::Serialize(Serializer& ser) const
  {
    Event::Serialize(ser);
    ser.write((unsigned)m_events.size());
    for (size_t i = 0; i < m_events.size(); ++i) {
      m_events[i]->Serialize(ser);
    }
  }

  const RawDataEvent& DetectorEvent::GetRawSubEvent(const std::string& subtype, int n) const
  {
    for (size_t i = 0; i < NumEvents(); i++) {
      const RawDataEvent* sev = dynamic_cast<const RawDataEvent*>(GetEvent(i));
      if (sev && sev->GetSubType() == subtype) {
        if (n > 0) {
          --n;
        } else {
          return *sev;
        }
      }
    }
    EUDAQ_THROW("DetectorEvent::GetRawSubEvent: could not find " + subtype + ":" + to_string(n));
  }

}
