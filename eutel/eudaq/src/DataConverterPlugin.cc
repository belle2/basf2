#include <eutel/eudaq/DataConverterPlugin.h>
#include <eutel/eudaq/PluginManager.h>

#include <iostream>

namespace eudaq {

  unsigned DataConverterPlugin::GetTriggerID(eudaq::Event const&) const
  {
    return (unsigned) - 1;
  }

  DataConverterPlugin::DataConverterPlugin(std::string subtype)
    : m_eventtype(make_pair(Event::str2id("_RAW"), subtype))
  {
    //std::cout << "DEBUG: Registering DataConverterPlugin: " << Event::id2str(m_eventtype.first) << ":" << m_eventtype.second << std::endl;
    PluginManager::GetInstance().RegisterPlugin(this);
  }

  DataConverterPlugin::DataConverterPlugin(unsigned type, std::string subtype)
    : m_eventtype(make_pair(type, subtype))
  {
    //std::cout << "DEBUG: Registering DataConverterPlugin: " << Event::id2str(m_eventtype.first) << ":" << m_eventtype.second << std::endl;
    PluginManager::GetInstance().RegisterPlugin(this);
  }

}//namespace eudaq
