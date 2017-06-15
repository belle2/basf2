#ifndef CDCTRIGGERETFModule_H
#define CDCTRIGGERETFModule_H

#include "framework/core/Module.h"
#include <string>

namespace Belle2 {

  /** Module for the Event Time Finder of the CDC trigger. */
  class CDCTriggerETFModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTriggerETFModule();

    /** Destructor */
    virtual ~CDCTriggerETFModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Run the ETF for an event. */
    virtual void event();

  protected:
    /** if true, always output 0 (assuming this is the true event time for MC) */
    bool m_trueEventTime;
    /** bin threshold for event time */
    unsigned m_threshold;
    /** name of the input track segment hit StoreArray */
    std::string m_hitCollectionName;
    /** name of the output StoreObjPtr holding the event time */
    std::string m_EventTimeName;

  private:
  };

} // namespace Belle2

#endif // CDCTriggerETFModule_H
