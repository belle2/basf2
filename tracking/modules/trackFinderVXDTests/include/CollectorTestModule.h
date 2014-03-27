#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /**
   * CollectorTestModules
   *
   */
  class CollectorTestModule : public Module {

  public:

    /** Constructor */
    CollectorTestModule();

    /** Init the module */
    virtual void initialize();
    /** Show progress */
    virtual void event();
    /** Don't break the terminal */
    virtual void terminate();

  protected:

  };
} // end namespace Belle2

