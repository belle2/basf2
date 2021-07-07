/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/DisplayData.h>
#include <G4NavigationHistory.hh>

#include <string>
#include <vector>
#include <set>

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

    /** Initialize the module. */
    void initialize() override;

    /** event function: this runs the overlap checker for each event */
    void event() override;

  private:

    /**
     * Check a volume for overlaps.
     *
     * Run the geant4 overlap check for the given volume and then call the overlap
     * check for all its daughters.
     *
     * @param volume The physical volume to be checked.
     * @param path The path name identifying the mother volume.
     * @param depth The current depth in this tree as this function is recursive
     * @return True if the volume or one of its daughters is overlapping
     */
    bool checkVolume(G4VPhysicalVolume* volume, const std::string& path, int depth = 0);

    /** Handle a G4Exception with the overlap message issued by Geant4 */
    void handleOverlap(const std::string& geant4Message);

    int    m_points{10000}; /**< number of test points */
    int    m_maxErrors{0}; /**< maximum number of errors before skipping current volume */
    int    m_maxDepth{0}; /**< maximum depth to check */
    double m_tolerance{0.}; /**< tolerance of overlap check */
    std::string m_prefix{""}; /**< check only volumes beginning with prefix */
    std::vector<std::string> m_overlaps;  /**< list of overlapping volumes */
    std::set<G4VPhysicalVolume*> m_seen; /**< set of logical volumes we already checked */
    G4NavigationHistory m_nav; /**< navigation history to remember coordinate transformations */
    StoreObjPtr<DisplayData> m_displayData; /**< Pointer to the DisplayData where we add the overlap points for rendering */
  };
} //Belle2 namespace
