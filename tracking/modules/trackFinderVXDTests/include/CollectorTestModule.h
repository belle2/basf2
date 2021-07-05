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
    void initialize() override;
    /** Show progress */
    void event() override;
    /** Don't break the terminal */
    void terminate() override;

  protected:

  };
} // end namespace Belle2

