#ifndef EUDAQ_INCLUDED_Event
#define EUDAQ_INCLUDED_Event

#include <string>
#include <vector>
#include <map>
#include <iosfwd>
#include <iostream>

#include <eutel/eudaq/Serializable.h>
#include <eutel/eudaq/Serializer.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/Utils.h>

#define EUDAQ_DECLARE_EVENT(type)           \
  public:                                   \
  static unsigned eudaq_static_id();      \
  virtual unsigned get_id() const {       \
    return eudaq_static_id();             \
  }                                       \
  private:                                  \
  static const int EUDAQ_DUMMY_VAR_DONT_USE = 0

#define EUDAQ_DEFINE_EVENT(type, id)       \
  unsigned type::eudaq_static_id() {       \
    static const unsigned id_(id);         \
    return id_;                            \
  }                                        \
  namespace _eudaq_dummy_ {                \
    static eudaq::RegisterEventType<type> eudaq_reg;  \
  }                                        \
  static const int EUDAQ_DUMMY_VAR_DONT_USE = 0

namespace eudaq {

  static const unsigned long long NOTIMESTAMP = (unsigned long long) - 1;

  class Event : public Serializable {
  public:
    enum Flags { FLAG_BORE = 1, FLAG_EORE = 2, FLAG_HITS = 4, FLAG_FAKE = 8, FLAG_SIMU = 16, FLAG_ALL = (unsigned) - 1 }; // Matches FLAGNAMES in .cc file
    Event(unsigned run, unsigned event, unsigned long long timestamp = NOTIMESTAMP, unsigned flags = 0)
      : m_flags(flags), m_runnumber(run), m_eventnumber(event), m_timestamp(timestamp) {}
    Event(Deserializer& ds);
    virtual void Serialize(Serializer&) const = 0;

    unsigned GetRunNumber() const { return m_runnumber; }
    unsigned GetEventNumber() const { return m_eventnumber; }
    unsigned long long GetTimestamp() const { return m_timestamp; }

    /** Returns the type string of the event implementation.
     *  Used by the plugin mechanism to identfy the event type.
     */
    virtual std::string GetSubType() const { return ""; }

    virtual void Print(std::ostream& os) const = 0;

    Event& SetTag(const std::string& name, const std::string& val);
    template <typename T>
    Event& SetTag(const std::string& name, const T& val)
    {
      return SetTag(name, eudaq::to_string(val));
    }
    std::string GetTag(const std::string& name, const std::string& def = "") const;
    std::string GetTag(const std::string& name, const char* def) const { return GetTag(name, std::string(def)); }
    template <typename T>
    T GetTag(const std::string& name, T def) const
    {
      return eudaq::from_string(GetTag(name), def);
    }

    bool IsBORE() const { return GetFlags(FLAG_BORE) != 0; }
    bool IsEORE() const { return GetFlags(FLAG_EORE) != 0; }
    bool HasHits() const { return GetFlags(FLAG_HITS) != 0; }
    bool IsFake() const { return GetFlags(FLAG_FAKE) != 0; }
    bool IsSimulation() const { return GetFlags(FLAG_SIMU) != 0; }

    static unsigned str2id(const std::string& idstr);
    static std::string id2str(unsigned id);
    unsigned GetFlags(unsigned f = FLAG_ALL) const { return m_flags & f; }
    void SetFlags(unsigned f) { m_flags |= f; }
    void ClearFlags(unsigned f = FLAG_ALL) { m_flags &= ~f; }
    virtual unsigned get_id() const = 0;
  protected:
    typedef std::map<std::string, std::string> map_t;

    unsigned m_flags, m_runnumber, m_eventnumber;
    unsigned long long m_timestamp;
    map_t m_tags; ///< Metadata tags in (name=value) pairs of strings
  };

  std::ostream& operator << (std::ostream&, const Event&);

  class EventFactory {
  public:
    static Event* Create(Deserializer& ds)
    {
      unsigned id = 0;
      ds.read(id);
      //std::cout << "Create id = " << std::hex << id << std::dec << std::endl;
      event_creator cr = GetCreator(id);
      if (!cr) EUDAQ_THROW("Unrecognised Event type (" + Event::id2str(id) + ")");
      return cr(ds);
    }

    typedef Event* (* event_creator)(Deserializer& ds);
    static void Register(unsigned long id, event_creator func);
    static event_creator GetCreator(unsigned long id);

  private:
    typedef std::map<unsigned long, event_creator> map_t;
    static map_t& get_map();
  };

  /** A utility template class for registering an Event type.
   */
  template <typename T_Evt>
  struct RegisterEventType {
    RegisterEventType()
    {
      EventFactory::Register(T_Evt::eudaq_static_id(), &factory_func);
    }
    static Event* factory_func(Deserializer& ds)
    {
      return new T_Evt(ds);
    }
  };
}

#endif // EUDAQ_INCLUDED_Event
