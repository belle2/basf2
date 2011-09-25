/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OVERLAPCHECKERMODULE_H
#define OVERLAPCHECKERMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <vector>

class G4VPhysicalVolume;


namespace Belle2 {

  /** The geometry overlap check module.
   *
   * This module goes through all volumes of the Belle II detector
   * geometry and checks them for overlaps. Overlaps are logged as errors.
   *
   * This module requires a valid geometry. Therefore,
   * a geometry building module should have been executed before this module is called.
   */
  class OverlapCheckerModule: public Module {

  public:

    /** Constructor of the module. */
    OverlapCheckerModule();

    /**
     * Initialize the module.
     *
     * This runs the overlap checker.
     */
    void initialize();

  private:

    /**
     * Check a volume for overlaps.
     *
     * Run the geant4 overlap check for the given volume and then call the overlap
     * check for all its daughters.
     *
     * @param volume The physical volume to be checked.
     * @param path The path name identifying the mother volume.
     * @return True if the volume or one of its daughters is overlapping
     */
    bool checkVolume(G4VPhysicalVolume* volume, const std::string& path);

    int    m_points;     /**< number of test points */
    double m_tolerance;  /**< tolerance of overlap check */
    std::vector<std::string> m_overlaps;  /**< list of overlapping volumes */
  };
} //Belle2 namespace
#endif
