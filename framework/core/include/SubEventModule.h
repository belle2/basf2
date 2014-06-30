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
#include <framework/core/Path.h>
#include <framework/core/EventProcessor.h>
#include <framework/datastore/StoreArray.h>


namespace Belle2 {
  /** Framework-internal module that implements the functionality of Path::forEach(). */
  class SubEventModule : public Module, public EventProcessor {
  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    SubEventModule(const std::string& objectName, const std::string& loopOver, boost::shared_ptr<Path> path);

    ~SubEventModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    std::string m_objectName; /**< name of our loop variable. */
    StoreArray<TObject> m_loopOver; /**< array looped over. */
    boost::shared_ptr<Path> m_path; /**< Path to execute. */

  };
}
