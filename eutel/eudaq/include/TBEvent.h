#ifndef TBEVENT_H
#define TBEVENT_H

#include <vector>
#include <map>
#include <memory>

#include "TBPlaneEvent.h"

class TBEvent {
public:
  TBEvent() : m_eventNo(0), m_runNo(0), m_timeStamp(0), m_mData(), m_Planes()
  {
    m_Planes.reset(new std::vector<unsigned short int>());
  }

  inline unsigned long long int getEventNumber() const { return m_eventNo; }
  inline unsigned long long int getRunNumber() const { return m_runNo; }
  inline unsigned long long int getTimeStamp() const { return m_timeStamp; }

  inline void setEventNumber(unsigned long long int evtNo) { m_eventNo = evtNo; }
  inline void setRunNumber(unsigned long long int runNo) { m_runNo = runNo; }
  inline void setTimeStamp(unsigned long long int timeStamp) { m_timeStamp = timeStamp; }

  void addPlaneEvent(unsigned short int iPl, const TBPlaneEvent& planeEvt)
  {
    // check whether we are overwriting an existing plane
    // if we are not overwriting an existing plane, add to vector of
    // plane numbers
    if (m_mData.find(iPl) == m_mData.end()) {
      m_Planes->push_back(iPl);
    }
    m_mData[ iPl ] = planeEvt;
  }

  std::shared_ptr<const std::vector<unsigned short int> > getPlaneNumbers() const { return m_Planes; }

  size_t getNumPlanes() const { return m_mData.size(); }
  bool getPlaneEvent(unsigned short int iPl, TBPlaneEvent& planeEvt) const
  {
    std::map<unsigned short int, TBPlaneEvent>::const_iterator pos = m_mData.find(iPl);
    if (pos != m_mData.end()) {
      planeEvt = pos->second;
      return true;
    } else {
      return false;
    }
  }
private:
  unsigned long long int m_eventNo;
  unsigned long long int m_runNo;
  unsigned long long int m_timeStamp;
  std::map<unsigned short int, TBPlaneEvent> m_mData;
  std::shared_ptr<std::vector<unsigned short int> > m_Planes;
};

#endif
