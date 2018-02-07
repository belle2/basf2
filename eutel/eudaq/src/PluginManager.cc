#include <eutel/eudaq/PluginManager.h>
#include <eutel/eudaq/Exception.h>
#include <eutel/eudaq/Configuration.h>

//#include <iostream>
#include <string>
using namespace std;

namespace eudaq {

  PluginManager& PluginManager::GetInstance()
  {
    // the only one static instance of the plugin manager is in the getInstance function
    // like this it is ensured that the instance is created before it is used
    static PluginManager manager;
    return manager;
  }

  void PluginManager::RegisterPlugin(DataConverterPlugin* plugin)
  {
    m_pluginmap[plugin->GetEventType()] = plugin;
  }

  DataConverterPlugin& PluginManager::GetPlugin(const Event& event)
  {
    return GetPlugin(std::make_pair(event.get_id(), event.GetSubType()));
  }

  DataConverterPlugin& PluginManager::GetPlugin(PluginManager::t_eventid eventtype)
  {
    std::map<t_eventid, DataConverterPlugin*>::iterator pluginiter
      = m_pluginmap.find(eventtype);

    if (pluginiter == m_pluginmap.end()) {
      EUDAQ_THROW("PluginManager::GetPlugin(): Unkown event type " + Event::id2str(eventtype.first) + ":" + eventtype.second);
    }

    return *pluginiter->second;
  }

  void PluginManager::Initialize(const DetectorEvent& dev)
  {
    const eudaq::Configuration conf(dev.GetTag("CONFIG"));
    for (size_t i = 0; i < dev.NumEvents(); ++i) {
      const eudaq::Event& subev = *dev.GetEvent(i);
      GetInstance().GetPlugin(subev).Initialize(subev, conf);
    }
  }

  unsigned PluginManager::GetTriggerID(const Event& ev)
  {
    return GetInstance().GetPlugin(ev).GetTriggerID(ev);
  }


  StandardEvent PluginManager::ConvertToStandard(const DetectorEvent& dev)
  {
    //StandardEvent event(dev.GetRunNumber(), dev.GetEventNumber(), dev.GetTimestamp());
    StandardEvent event(dev);
    for (size_t i = 0; i < dev.NumEvents(); ++i) {
      const Event* ev = dev.GetEvent(i);
      if (!ev) EUDAQ_THROW("Null event!");
      if (ev->GetSubType() == "EUDRB") {
        ConvertStandardSubEvent(event, *ev);
      }
    }
    for (size_t i = 0; i < dev.NumEvents(); ++i) {
      const Event* ev = dev.GetEvent(i);
      if (!ev) EUDAQ_THROW("Null event!");
      if (ev->GetSubType() != "EUDRB") {
        ConvertStandardSubEvent(event, *ev);
      }
    }
    return event;
  }

  TBTelEvent PluginManager::ConvertToTBTelEvent(const DetectorEvent& dev)
  {
    TBTelEvent tbEvent;
    tbEvent.setEventNumber(dev.GetEventNumber());
    tbEvent.setRunNumber(dev.GetRunNumber());
    tbEvent.setTimeStamp(dev.GetTimestamp());

    for (size_t i = 0; i < dev.NumEvents(); ++i) {
      ConvertTBTelEventSubEvent(tbEvent, * dev.GetEvent(i));
    }

    return tbEvent;
  }

  void PluginManager::ConvertStandardSubEvent(StandardEvent& dest, const Event& source)
  {
    try {
      GetInstance().GetPlugin(source).GetStandardSubEvent(dest, source);
    } catch (const Exception& e) {
      std::cerr << "Error during conversion in PluginManager::ConvertStandardSubEvent:\n" << e.what() << std::endl;
    }
  }

  void PluginManager::ConvertTBTelEventSubEvent(TBTelEvent& dest, const Event& source)
  {

    GetInstance().GetPlugin(source).GetTBTelEventSubEvent(dest, source);

  }

}//namespace eudaq
