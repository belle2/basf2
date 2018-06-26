/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <tuple>
#include <TObject.h>
#include <iostream>

//number of channels per HAPD
#define N_MODULES 420

namespace Belle2 {
  //! The Class for ARICH mapping of bias cables to modules
  /*!
    This class provides ARICH mapping of HAPD modules to bias cables. It is assumed sectorID going from 1:6, cableID goes from 1:8 and innerID goes from 1:9.
  */
  class ARICHBiasCablesMapping : public TObject {

  public:

    //! Default constructor
    ARICHBiasCablesMapping();

    /**
     * Get ID of module connected to cable in sector
     * @param sectorID sector ID
     * @param cableID  bias cable number
     * @param innerID inner cable number
     * @return moduleID module ID number
     */
    int getModuleID(int sectorID, int cableID, int innerID) const;

    /**
     * Get ID of sector of a cable to which module is connected
     * @param moduleID module ID number
     * @return sectorID sector ID number
     */
    int getSectorID(int moduleID) const;

    /**
     * Get ID of cable to which module is connected
     * @param moduleID module ID number
     * @return cableID cable ID number
     */
    int getCableID(int moduleID) const;

    /**
     * Get inner cable ID to which module is connected
     * @param moduleID module ID number
     * @return innerID inner ID number
     */
    int getInnerID(int moduleID) const;

    /**
     * Add mapping entry
     * @param moduleID module ID number
     * @param sectorID sector ID
     * @param cableID  bias cable number
     * @param innerID inner cable number
     */
    void addMapping(int moduleID, int sectorID, int cableID, int innerID);

    /**
     * Pring mapping
     */
    void print() const;

  private:

    std::vector<std::tuple<int, int, int>> m_cable2module; /**< map of cables to modules */
    std::map<int, std::vector<int>> m_module2cable; /**< map of modules to cables */

    ClassDef(ARICHBiasCablesMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
