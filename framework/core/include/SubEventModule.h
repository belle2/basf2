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
#include <boost/optional.hpp>
#include <memory>

namespace Belle2 {
  class Path;
  /** Framework-internal module that implements the functionality of
   * Path::forEach() as well as Path::doWhile(). */
  class SubEventModule : public Module, public EventProcessor {
  public:
    /** Define the constants for the different modes */
    enum EModes {
      c_ForEach = 0,
      c_DoWhile = 1,
    };

    SubEventModule();
    ~SubEventModule();

    /** used by Path::forEach() to actually set parameters. */
    void initSubEvent(const std::string& objectName, const std::string& loopOver, std::shared_ptr<Path> path);
    /** ised by Path::doWhile() to actually set parameters */
    void initSubLoop(std::shared_ptr<Path> path, const std::string& condition, unsigned int maxIterations);

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    /** Set properties for this module based on the modules found in m_path */
    void setProperties();
    /** Set the necessary pointers for do_while(): the pointer to the module
     * whose return value we'll use as well as the ModuleCondition object.
     * Does nothing if both are setup already */
    void setDoWhileConditions();
    /** name of our loop variable in case of forEach. */
    boost::optional<std::string> m_objectName{boost::none};
    /** name for m_loopOver in case of forEach. */
    boost::optional<std::string> m_loopOverName{boost::none};
    /** array looped over in case of forEach */
    StoreArray<TObject> m_loopOver;
    /** Path to execute. */
    std::shared_ptr<Path> m_path;
    /** when using multi-processing contains the ID of the process where
     * event() is called (in that process only). -1 otherwise. */
    int m_processID{ -1};
    /** maximum number of iterations before giving up in case of doWhile() */
    unsigned int m_maxIterations{10000};
    /** String for the condition when looping */
    boost::optional<std::string> m_loopConditionString{boost::none};
    /** Condition object to evaluate if the loop is finished in case of doWhile() */
    std::unique_ptr<ModuleCondition> m_loopCondition;
    /** pointer to the module to provide the returnValue in case of doWhile() */
    Module* m_loopConditionModule{nullptr};
    /** Mode for this module. Should be 0=for_each, 1=do_while */
    int m_mode{c_ForEach};
  };
}
