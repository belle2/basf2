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

  private:
  };

} // namespace Belle2

#endif // CDCTriggerETFModule_H
