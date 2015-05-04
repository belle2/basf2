/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jeter Hall,                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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

    /* The global tag */
    std::string m_globalTag;
    std::string m_file_local;

    /* The experiment name */
    std::string m_experimentName;

    /* The run name */
    std::string m_runName;

    /* The location of the REST services (ie http://belle2db.hep.pnnl.gov/) */
    std::string m_restBasename;

    /* The location of the conditions files (ie /cvmfs/belle2/conditions/ ) */
    std::string m_fileBasename;

    /* A temporary location to store files if the files need to be downloaded */
    std::string m_fileLocal;


    /// For manual additions to the conditions database
    /* file name for manual addition of a payload to the database */
    std::string m_filename;
    /* package name for manual addition of a payload to the database */
    std::string m_package;
    /* module name for manual addition of a payload to the database */
    std::string m_module;
    /* comment for manual addition to database.  This is not currently used. */
    std::string m_comment;

  };
}
#endif
