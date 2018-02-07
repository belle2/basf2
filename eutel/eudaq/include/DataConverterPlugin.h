#ifndef EUDAQ_INCLUDED_DataConverterPlugin
#define EUDAQ_INCLUDED_DataConverterPlugin

#include <eutel/eudaq/StandardEvent.h>
#include <eutel/eudaq/RawDataEvent.h>
#include <eutel/eudaq/TBTelEvent.h>

#include <string>
#include <algorithm>

namespace eudaq {

  class Configuration;

  /**
   *  The DataConverterPlugin is the abstract base for all plugins. The
   *  actual impementation provides the GetTBTelEventEvent() and GetStandardEvent()
   *  functions which
   *  get the eudaq::Event as input parameter.
   *
   *  The implementations should be sinlgeton classes which only can be
   *  accessed via the plugin manager. (See TimepixConverterPlugin as example).
   *  The plugin implementations have to register with the plugin manager.
   */

  class DataConverterPlugin {
  public:
    typedef std::pair<unsigned, std::string> t_eventid;

    virtual void Initialize(eudaq::Event const&, eudaq::Configuration const&) {}

    virtual unsigned GetTriggerID(eudaq::Event const&) const;

    /** Returns the TBTelEvent version of the event.
     */
    virtual bool GetTBTelEventSubEvent(TBTelEvent& /*result*/, eudaq::Event const& /*source*/) const { return false; }

    /** Returns the StandardEvent version of the event.
     */
    virtual bool GetStandardSubEvent(StandardEvent& /*result*/, eudaq::Event const& /*source*/) const { return false; };

    /** Returns the type of event this plugin can convert to lcio as a pair of Event type id and subtype string.
     */
    virtual t_eventid const& GetEventType() const { return m_eventtype; }

    /** The empty destructor. Need to add it to make it virtual.
     */
    virtual ~DataConverterPlugin() {}

  protected:
    /** The string storing the event type this plugin can convert to TBTelEvent.
     *  This string has to be set in the constructor of the actual implementations
     *  of the plugin.
     */
    t_eventid m_eventtype;

    /** The protected constructor which automatically registeres the plugin
     *  at the pluginManager.
     */
    DataConverterPlugin(std::string subtype);
    DataConverterPlugin(unsigned type, std::string subtype = "");
  private:
    /** The private copy constructor and assignment operator. They are not used anywhere, so there is not
     *  even an implementation. Even if the childs default copy constructor is public
     *  the code will not compile if it is called, since it cannot acces this cc, which the
     *  the default cc does.
     */
    DataConverterPlugin(DataConverterPlugin&);
    DataConverterPlugin& operator = (const DataConverterPlugin&);
  };

}//namespace eudaq

#endif // EUDAQ_INCLUDED_DataConverterPlugin
