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
// includes - stl:
#include <string>
#include <set>

// includes - tf-related stuff:
#include "SectorMap.h"

// includes - general fw stuff:


namespace Belle2 {


  /**SectorMapFactory - creates and handles a specific sectorMap.
   * - imports it from a xml file carrying a root stream
   * - reads relations and creates internal links for faster access between related objects
   **/
  class SectorMapFactory {
  public:

    /** constructor */
    SectorMapFactory():
      m_sectorMapName("std") { initialize(); }

    /** constructor */
    SectorMapFactory(std::string mapName):
      m_sectorMapName(mapName) { initialize(); }

    /** called once per run - imports the root-Version of the sectorMap. */
    void importRootMap();

    /** returns name of sectorMap for this factory */
    const std::string& getMapName() const { return m_sectorMapName; }

  protected:
    /** called by the constructors, prepares all the storeArrays. */
    void initialize();


    /** carries the name of the sectorMap, which is also the name of the storeArrays for the Classes connected to this secMap */
    std::string m_sectorMapName;


  };
} //Belle2 namespace