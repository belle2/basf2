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
#include <TObject.h>
#include <map>
#include <vector>

namespace Belle2 {
  //! The Class for ARICH bias voltages at gain 40
  /*!     This class provides ARICH bias voltages at gain 40
   */

  class ARICHHvCrateCableMapping : public TObject {

  public:

    //! Default constructor
    ARICHHvCrateCableMapping();

    /**
    * Get sector ID
     * @param connectionID connection ID
     * @return sector sector number ID
    */
    int getSector(int connectionID) const;

    /**
    * Get sector ID
     * @param connectionID connection ID
     * @return cable cable number ID
    */
    int getCable(int connectionID) const;

    /**
    * Get sector ID
    * @param sectorCable vector of sector and cable numbers
    * @return connectionID connection ID
    */
    int getConnectionID(std::vector<int> sectorCable) const;

    /**
     * Add new entry to the mapping table
     * @param crateSlot numbers of crate and slot
     * @param sectorCable sector and cable number
     */
    void addMapping(int connectionID, std::vector<int> sectorCable);

    void print();

  private:

    std::map<int, std::vector<int>> m_connection2cable; /**< map of modules to cables */

    ClassDef(ARICHHvCrateCableMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
