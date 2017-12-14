#include <eutel/eudaq/TransportBase.h>
#include <ostream>
#include <iostream>

namespace eudaq {

  const ConnectionInfo ConnectionInfo::ALL("ALL");
  static const int DEFAULT_TIMEOUT = 1000;

  void ConnectionInfo::Print(std::ostream& os) const
  {
    if (m_state == -1) {
      os << "(disconnected)";
    } else if (m_state == 0) {
      os << "(waiting)";
    } else {
      os << m_type;
      if (m_name != "") os << "." << m_name;
    }
  }

  bool ConnectionInfo::Matches(const ConnectionInfo& /*other*/) const
  {
    //std::cout << " [Match: all] " << std::flush;
    return true;
  }

  TransportBase::TransportBase()
    : m_callback(0)
  {}

  void TransportBase::SetCallback(const TransportCallback& callback)
  {
    m_callback = callback;
  }

  void TransportBase::Process(int timeout)
  {
    if (timeout == -1) timeout = DEFAULT_TIMEOUT;
    MutexLock m(m_mutex);
    ProcessEvents(timeout);
    m.Release();
    for (;;) {
      MutexLock m(m_mutex);
      if (m_events.empty()) break;
      //std::cout << "Got packet" << std::endl;
      TransportEvent evt(m_events.front());
      m_events.pop();
      m.Release();
      m_callback(evt);
    }
  }

  bool TransportBase::ReceivePacket(std::string* packet, int timeout, const ConnectionInfo& conn)
  {
    if (timeout == -1) timeout = DEFAULT_TIMEOUT;
    //std::cout << "ReceivePacket()" << std::flush;
    while (!m_events.empty() && m_events.front().etype != TransportEvent::RECEIVE) {
      m_events.pop();
    }
    //std::cout << " current level=" << m_events.size() << std::endl;
    if (m_events.empty()) {
      //std::cout << "Getting more" << std::endl;
      ProcessEvents(timeout);
      //std::cout << "Temp level=" << m_events.size() << std::endl;
      while (!m_events.empty() && m_events.front().etype != TransportEvent::RECEIVE) {
        m_events.pop();
      }
      //std::cout << "New level=" << m_events.size() << std::endl;
    }
    bool ret = false;
    if (!m_events.empty() && conn.Matches(m_events.front().id)) {
      ret = true;
      *packet = m_events.front().packet;
      m_events.pop();
    }
    //std::cout << "ReceivePacket() return " << (ret ? "true" : "false") << std::endl;
    return ret;
  }

  bool TransportBase::SendReceivePacket(const std::string& sendpacket,
                                        std::string* recpacket,
                                        const ConnectionInfo& connection,
                                        int timeout)
  {
    // acquire mutex...
    if (timeout == -1) timeout = DEFAULT_TIMEOUT;
    //std::cout << "DEBUG: SendReceive: Acquiring mutex" << std::endl;
    MutexLock m(m_mutex, false);
    try {
      m.Lock();
    } catch (const eudaq::Exception&) {
      // swallow it
    }
    //std::cout << "DEBUG: SendReceive: got mutex" << std::endl;
    SendPacket(sendpacket, connection);
    //std::cout << "DEBUG: SendReceive sent packet " << sendpacket << std::endl;
    bool ret = ReceivePacket(recpacket, timeout, connection);
    //std::cout << "SendReceivePacket() return '" << *recpacket << "' " << (ret ? "true" : "false") << std::endl;
    return ret;
  }

  TransportBase::~TransportBase()
  {
  }

}
