/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Umberto Tamponi                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <Python.h>
#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Clock database importer.
   * This module writes data to database or retrieves them from database.
   * More functions will come with future...
   */
  class ClockDatabaseImporter {

  public:

    /**
     * Default constructor
     */
    ClockDatabaseImporter()
    {}


    /**
     * Destructor
     */
    virtual ~ClockDatabaseImporter()
    {}

    /**
     * import a dummy payload of TOPCalTimebase DB objects
     * @param firstExp first experiment number of IOV
     * @param firstRun first run number of IOV
     * @param lastExp first experiment number of IOV
     * @param lastRun last run number of IOV
     */
    void importClock(int firstExp = 0, int firstRun = 0,
                     int lastExp = -1, int lastRun = -1);

  private:

    // ClassDef(ClockDatabaseImporter, 1);  /**< ClassDef - why this is needed? */
  };

} // Belle2 namespace
