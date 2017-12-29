/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/core/EventProcessor.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {
  class Path;
  /** Framework-internal module that implements the functionality of Path::forEach(). */
  class SubEventModule : public Module, public EventProcessor {
  public:

    SubEventModule();
    ~SubEventModule();

    /** used by forEach() to actually set parameters. */
    void initSubEvent(const std::string& objectName, const std::string& loopOver, std::shared_ptr<Path> path);

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    std::string m_objectName; /**< name of our loop variable. */
    std::string m_loopOverName; /**< name for m_loopOver. */
    StoreArray<TObject> m_loopOver; /**< array looped over. */
    std::shared_ptr<Path> m_path; /**< Path to execute. */
    int m_processID; /**< when using multi-processing contains the ID of the process where event() is called (in that process only). -1 otherwise. */

  };
}
