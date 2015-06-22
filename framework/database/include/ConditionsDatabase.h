/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/database/Database.h>


namespace Belle2 {
  /**
   * Database backend that uses the conditions service.
   */
  class ConditionsDatabase: public Database {
  public:

    /**
     * Method to set the database instance to the central database with default parameters.
     */
    static void createDefaultInstance();

    /**
     * Method to set the database instance to the central database.
     *
     * @param restBaseName   Base name for REST services
     * @param fileBaseName   Base name for conditions files
     * @param fileBaseLocal  Directory name for local conditions files copies
     */
    static void createInstance(const std::string& restBaseName, const std::string& fileBaseName, const std::string& fileBaseLocal);

    /**
     * Request an object from the database.
     *
     * @param event      The metadata of the event for which the object should be valid.
     * @param name       Name that identifies the object in the database.
     * @return           A pair of a pointer to the object and the interval for which it is valid
     */
    virtual std::pair<TObject*, IntervalOfValidity> getData(const EventMetaData& event, const std::string& name);

    /**
     * Store an object in the database.
     *
     * @param name       Name that identifies the object in the database.
     * @param object     The object that should be stored in the database.
     * @param iov        The interval of validity of the the object.
     * @return           True if the storage of the object succeeded.
     */
    virtual bool storeData(const std::string& name, TObject* object, IntervalOfValidity& iov);

  private:
    /** Hidden constructor, as it is a singleton. */
    explicit ConditionsDatabase(): Database(""), m_currentExperiment((unsigned long) - 1), m_currentRun(0) {};

    /** Hidden copy constructor, as it is a singleton. */
    ConditionsDatabase(const ConditionsDatabase&);

    /** Hidden destructor, as it is a singleton. */
    virtual ~ConditionsDatabase() {};

    /** Experiment number for which the payloads were obtained. */
    unsigned long m_currentExperiment;

    /** Run number for which the payloads were obtained. */
    unsigned long m_currentRun;
  };
} // namespace Belle2
