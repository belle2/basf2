#include <testbeam/vxd/dataobjects/TelEventInfo.h>

using namespace Belle2;

ClassImp(TelEventInfo)

TelEventInfo::TelEventInfo(ULong64_t event /* = 0 */,
                           ULong64_t triggerID /* = 0 */,
                           ULong64_t timeStamp /* = 0 */)
  : m_event(event),
    m_triggerID(triggerID),
    m_timeStamp(timeStamp)
{

}

bool TelEventInfo::operator==(const TelEventInfo& telEvtInfo) const
{
  return ((m_event == telEvtInfo.getEventNumber()) &&
          (m_triggerID == telEvtInfo.getTriggerID()) &&
          (m_timeStamp == telEvtInfo.getTimeStamp()));
}

bool TelEventInfo::operator!=(const TelEventInfo& telEvtInfo) const
{
  return !(* this == telEvtInfo);
}
