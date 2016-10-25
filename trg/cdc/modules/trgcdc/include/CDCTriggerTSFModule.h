#ifndef CDCTRIGGERTSFModule_H
#define CDCTRIGGERTSFModule_H

#include "framework/core/Module.h"
#include <string>

namespace Belle2 {

  /** Module for the Track Segment Finder of the CDC trigger. */
  class CDCTriggerTSFModule : public Module {

  public:

    /** Constructor, for setting module description and parameters. */
    CDCTriggerTSFModule();

    /** Destructor */
    virtual ~CDCTriggerTSFModule() {}

    /** Initialize the module and register DataStore arrays. */
    virtual void initialize();

    /** Run the TSF for an event. */
    virtual void event();

  protected:
    /** The filename of LUT for the inner-most track segments. */
    std::string m_innerTSLUTFilename;

    /** The filename of LUT for outer track segments. */
    std::string m_outerTSLUTFilename;

  private:
  };

} // namespace Belle2

#endif // CDCTriggerTSFModule_H
