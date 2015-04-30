#ifndef ConditionsModule_H
#define ConditionsModule_H

#include <framework/core/Module.h>

namespace Belle2 {
  /**  The ConditionsModule initializes the ConditionsService.
   *
   *   This module is under development and should not be used.
   *
   */

  class ConditionsModule : public Module {

  public:

    /** The Constructor. */
    ConditionsModule();

    /** Initialize creates the first instance of the ConditionsService. */
    void initialize();

    /** Deal with any run dependance for the module.  Modules using the ConditionsService will have to
     *  deal with their own run dependance.
     */
    void beginRun();



    /** The Terminator. */
    void terminate();


  private:

    //Configuration  parameters to setup dB
    std::string m_global_tag;
    std::string m_experiment_name;
    std::string m_run_name;
    std::string m_rest_basename;
    std::string m_file_basename;
    std::string m_file_local;

    //Parameters for manual payload addition
    std::string m_filename;
    std::string m_package;
    std::string m_module;
    std::string m_comment;

  };
}
#endif
