/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <TObject.h>
#include <map>
#include <vector>

#define N_COPPERS 18

namespace Belle2 {
  //! The Class for ARICH mapping of merger to copper boards
  /*! This class provides ARICH mapping of merger to copper boards
   *  Cooper ID can be arbitrary number, finesse number goes from 0-3
   *  Merger ID goes from 1 to 72
   */

  class ARICHCopperMapping : public TObject {

  public:

    //! Default constructor
    ARICHCopperMapping();

    /**
     * Get copper board to which merger is connected
     * @param mergerID ID of merger board
     * @return copper board ID
     */
    unsigned getCopperID(unsigned mergerID) const;

    /**
     * Get finesse of copper board to which merger is connected
     * @param mergerID ID of merger board
     * @return finesse number
     */
    unsigned getFinesse(unsigned mergerID) const;


    /**
     * Add new entry to the mapping table
     * @param mergerID ID of merger board
     * @param copperID copper board ID number
     * @param finesse finesse number
     */
    void addMapping(unsigned mergerID, unsigned copperID, unsigned finesse);

    /**
    * Get ID of merger connected to the given finesse of copper board
    * @param copperID copper board ID number
    * @param finesse finesse number
    * @return merger board ID
    */
    unsigned getMergerID(unsigned copperID, unsigned finesse) ;

    /**
     * Get vector of IDs of copper boards
     * @return vector of copper IDs
     */
    const std::vector<uint16_t>& getCopperIDs() const { return m_copperIDs;}

    /**
    * Print mapping parameters
    */
    void print();

  private:

    std::vector<std::pair<uint16_t, uint16_t>> m_merger2copper; /**< map of mergers to copper boards */

    std::map<uint16_t, std::vector<uint16_t>> m_copper2merger;  /**< map coppers to mergers */

    std::vector<uint16_t> m_copperIDs;   /**< vector of IDs of used copper boards */

    ClassDef(ARICHCopperMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
