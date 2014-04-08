/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// includes - rootStuff:
#include <TVector3.h>

// includes - stl:
#include <vector>
#include <utility> // std::pair

// includes - tf-related stuff:
// includes - general fw stuff:
#include <framework/core/FrameworkExceptions.h>


namespace Belle2 {


  /**SectorTools - contains often needed transformer-functions.
   * -
   **/
  class SectorTools {
  public:
    /** .first is uValue, .second is vValue (both defined between 0-1) */
    typedef std::pair<float, float> RelCoords;

    /** shortcut for unsigned int short */
    typedef unsigned short uShort;

    BELLE2_DEFINE_EXCEPTION(Out_of_bounds, "The relative coordinates (sectorConfig or coordinates themselves) are not defined between 0-1!");

    /** calculates the sector ID for current hit.
     * for given configuration for u and v coordinates, it takes a hit in relative coordinates (sensor-independent) and calculates the sectorID */
    static uShort calcSecID(const std::vector<float>& uConfig, const std::vector<float>& vConfig, RelCoords coords) {
      return 0; /*dummy*/
    }

    /** for a given point  */
    static RelCoords calcRelativeSectorPoint(const std::vector<float>& uConfig, const std::vector<float>& vConfig, uShort secID, RelCoords coords) {
      return make_pair(0, 0); /*dummy*/
    }

  protected:
  };
} //Belle2 namespace