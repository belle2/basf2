#ifndef DEPFET_ADCVALUES_H
#define DEPFET_ADCVALUES_H

#include <stdexcept>
#include <stdint.h>

namespace depfet {

  class DEPFETADCValues: public std::vector< std::vector<uint16_t> > {
  public:
    DEPFETADCValues(): std::vector< std::vector<uint16_t> >(4, std::vector<uint16_t>(0, 0)), m_moduleNr(0), m_triggerNr(0),
      m_startGate(-1) {}
    int getModuleNr() const { return m_moduleNr; }
    int getTriggerNr() const { return m_triggerNr; }
    int getStartGate() const { return m_startGate; }
    void setModuleNr(int moduleNr) { m_moduleNr = moduleNr; }
    void setTriggerNr(int triggerNr) { m_triggerNr = triggerNr; }
    void setStartGate(int startGate) { m_startGate = startGate; }
  protected:
    int m_moduleNr;
    int m_triggerNr;
    int m_startGate;
  };

}

#endif


