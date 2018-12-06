#ifndef DEPFET_EVENT_H
#define DEPFET_EVENT_H

#include <eutel/eudaq/DEPFETADCValues.h>
#include <vector>

namespace depfet {
  class DEPFETEvent: public std::vector<DEPFETADCValues> {
  public:
    DEPFETEvent(int nModules = 0): std::vector<DEPFETADCValues>(nModules), m_runNumber(0), m_eventNumber(0) {}

    int getRunNumber() const { return m_runNumber; }
    int getEventNumber() const { return m_eventNumber; }

    void setRunNumber(int runNumber) { m_runNumber = runNumber; }
    void setEventNumber(int eventNumber) { m_eventNumber = eventNumber; }

    void clear()
    {
      for (iterator it = begin(); it != end(); ++it) it->clear();
    }
  protected:
    int m_runNumber;
    int m_eventNumber;
  };
}

#endif
