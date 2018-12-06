#ifndef EUDAQ_INCLUDED_TransportBase
#define EUDAQ_INCLUDED_TransportBase

/** \file TransportBase.hh
 * Defines TransportBase along with a number of related classes.
 */

#include <eutel/eudaq/Mutex.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/BufferSerializer.h>
#include <string>
#include <queue>
#include <iosfwd>
#include <cstring>
#include <iostream>

namespace eudaq {

  /** Represents an individual connection on a Transport.
   *
   */
  class ConnectionInfo {
  public:
    explicit ConnectionInfo(const std::string& name = "") : m_state(0), m_name(name) {}
    virtual ~ConnectionInfo() {}
    virtual void Print(std::ostream&) const;
    virtual bool Matches(const ConnectionInfo& other) const;
    bool IsEnabled() const { return m_state >= 0; }
    int GetState() const { return m_state; }
    void SetState(int state) { m_state = state; }
    std::string GetType() const { return m_type; }
    void SetType(const std::string& type) { m_type = type; }
    std::string GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }
    virtual std::string GetRemote() const { return ""; }
    static const ConnectionInfo ALL;

    virtual ConnectionInfo* Clone() const { return new ConnectionInfo(*this); }
  protected:
    int m_state;
    std::string m_type, m_name;
    /*
    public:
    virtual bool operator = (const ClientID & other) = 0;
    virtual std::string Name() const = 0;
    */
  };

  inline std::ostream& operator << (std::ostream& os, const ConnectionInfo& id)
  {
    id.Print(os);
    return os;
  }

  /** Represents an event such as a connection, or receipt of data on a Transport.
   */
  struct TransportEvent {
    enum EventType { CONNECT, DISCONNECT, RECEIVE };
    TransportEvent(EventType et, ConnectionInfo& i, const std::string& p = "")
      : etype(et), id(i), packet(p) {}
    EventType etype;    ///< The type of event
    ConnectionInfo& id;     ///< The id of the connection
    std::string packet; ///< The packet of data in case of a RECEIVE event
  };

  /** Represents a callback function for the Transport system.
   * It can hold a pointer to a callback function which can be
   * either a free function or an object and a member function.
   * It is like a simplified and less flexible version of boost::function.
   * I didn't use boost::function because I didn't want to rely
   * on too many external dependencies.
   */
  class TransportCallback {
    /** A helper class for TransportCallback.
     * It specifies an interface from which the actual helpers inherit.
     */
    class Helper {
    public:
      virtual void call(TransportEvent&) = 0;
      virtual ~Helper() { }
      virtual Helper* Clone() const = 0;
    };
    /** A helper class for TransportCallback for normal function pointers.
     * It performs the actual function call for a normal function pointer.
     */
    class HelperFunction : public Helper {
    public:
      typedef void (*FuncType)(TransportEvent&);
      HelperFunction(FuncType func) : m_func(func) { }
      virtual void call(TransportEvent& ev) { m_func(ev); }
      virtual Helper* Clone() const { return new HelperFunction(*this); }
    private:
      FuncType m_func; ///< A pointer to the function to call
    };
    /** A helper class for TransportCallback for member function pointers.
     * It performs the actual function call for a member function pointer.
     * It also stores a pointer to the object on which to call the member function.
     */
    template <typename T>
    class HelperMember : public Helper {
    public:
      typedef T ObjType;
      typedef void (ObjType::*FuncType)(TransportEvent&);
      HelperMember(ObjType* obj, FuncType func) : m_obj(obj), m_func(func) {}
      virtual void call(TransportEvent& ev)
      {
        (m_obj->*m_func)(ev);
      }
      virtual Helper* Clone() const { return new HelperMember(*this); }
    private:
      ObjType* m_obj; ///< A pointer to the object
      FuncType m_func; ///< A pointer to the member function
    };
  public:
    TransportCallback() : m_helper(0) {}
    TransportCallback(HelperFunction::FuncType funcptr)
      : m_helper(new HelperFunction(funcptr)) {}
    template <typename T>
    TransportCallback(T* obj, typename HelperMember<T>::FuncType func)
      : m_helper(new HelperMember<T>(obj, func)) {}
    void operator()(TransportEvent& ev)
    {
      if (m_helper) m_helper->call(ev);
    }
    TransportCallback(const TransportCallback& other)
      : m_helper(other.m_helper->Clone()) {}
    TransportCallback& operator=(const TransportCallback& other)
    {
      delete m_helper;
      m_helper = other.m_helper->Clone();
      return *this;
    }
    ~TransportCallback() { delete m_helper; }
  private:
    Helper* m_helper;  ///< The helper class to perform the actual function call
  };

  /** A base class from which all types of Transport should inherit.
   * They must implement the two pure virtual member functions SendPacket and ProcessEvents.
   * A Transport is a means of transferring data or commands from one
   * process to another, possibly on another machine.
   */
  class TransportBase {
  public:
    TransportBase();
    virtual ~TransportBase();

    /** Pure virtual function to send a packet of data.
     * This function must be implemented by the concrete Transport class to send a
     * packet of data to the remote Transport instance.
     */
    virtual void SendPacket(const unsigned char* data, size_t len,
                            const ConnectionInfo& = ConnectionInfo::ALL,
                            bool duringconnect = false) = 0;
    void SendPacket(const std::string& data, const ConnectionInfo& inf = ConnectionInfo::ALL, bool duringconnect = false)
    {
      //std::cout << "SendPacket (string)" << std::endl;
      SendPacket(reinterpret_cast<const unsigned char*>(&data[0]), data.length(), inf, duringconnect);
    }
    void SendPacket(const char* str, const ConnectionInfo& inf = ConnectionInfo::ALL, bool duringconnect = false)
    {
      //std::cout << "SendPacket (char*)" << std::endl;
      SendPacket(reinterpret_cast<const unsigned char*>(str), std::strlen(str), inf, duringconnect);
    }
    void SendPacket(const BufferSerializer& t, const ConnectionInfo& inf = ConnectionInfo::ALL, bool duringconnect = false)
    {
      SendPacket(&t[0], t.size(), inf, duringconnect);
    }

    /** Pure virtual function to close a connection.
     * This function should be implemented by the concrete Transport class to close
     * the connection to the specified remote Transport instance.
     */
    virtual void Close(const ConnectionInfo&) {}

    /** Pure virtual function to receive data.
     * This function must be implemented by the concrete Transport class to receive
     * all pending data from the remote Transport instance, and fill the queue of
     * events so that they may be handled.
     */
    virtual void ProcessEvents(int timeout) = 0;

    void Process(int timeout = -1);
    bool ReceivePacket(std::string* packet, int timeout = -1, const ConnectionInfo& = ConnectionInfo::ALL);
    bool SendReceivePacket(const std::string& sendpacket,
                           std::string* recpacket,
                           const ConnectionInfo& connection,
                           int timeout = -1);
    template <typename T>
    T SendReceivePacket(const std::string& packet, const ConnectionInfo& connection, int timeout = -1);
    void SetCallback(const TransportCallback&);
    virtual bool IsNull() const { return false; }
  protected:
    std::queue<TransportEvent> m_events; ///< A buffer to queue up events until they are handled
    TransportCallback m_callback; ///< The callback function to invoke on a transport event
    Mutex m_mutex;
  };

  template <typename T>
  inline T TransportBase::SendReceivePacket(const std::string& packet, const ConnectionInfo& connection, int timeout)
  {
    std::string buf;
    bool ok = SendReceivePacket(packet, &buf, connection, timeout);
    if (!ok) EUDAQ_THROW("Timeout: No response in SendReceivePacket");
    BufferSerializer ser(buf.begin(), buf.end());
    T result;
    ser.read(result);
    return result;
  }


}

#endif // EUDAQ_INCLUDED_TransportBase
