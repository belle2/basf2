#include "daq/slc/hvcontrol/HVConfig.h"

using namespace Belle2;

void HVConfig::reset() throw()
{
  m_channel_v = HVChannelList();
  m_valueset_v = HVValueSetList();
}

void HVConfig::set(const ConfigObject& obj) throw()
{
  reset();
  m_obj = obj;
  {
    ConfigObjectList& obj_v(m_obj.getObjects("channel"));
    for (size_t i = 0; i < obj_v.size(); i++) {
      m_channel_v.push_back(HVChannel(&obj_v[i]));
    }
  }
  {
    ConfigObjectList& obj_v(m_obj.getObjects("valueset"));
    for (size_t i = 0; i < obj_v.size(); i++) {
      HVValueSet value_v;
      ConfigObjectList& cobj_v(obj_v[i].getObjects("value"));
      for (size_t j = 0; j < cobj_v.size(); j++) {
        value_v.push_back(HVValue(&cobj_v[j]));
      }
      m_valueset_v.push_back(value_v);
    }
  }

}
