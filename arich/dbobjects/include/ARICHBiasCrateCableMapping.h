/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <map>
#include <vector>

namespace Belle2 {

  /**
   * This class provides ARICH bias voltages at gain 40
   */
  class ARICHBiasCrateCableMapping : public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHBiasCrateCableMapping();

    /**
     * Get sector ID.
     * @param[in] connectionID Connection ID.
     * @return Sector number ID.
     */
    int getSector(int connectionID) const;

    /**
     * Get cable ID.
     * @param[in] connectionID Connection ID.
     * @return cable Cable number ID.
     */
    int getCable(int connectionID) const;

    /**
     * Get connection ID.
     * @param[in] sectorCable Vector of sector and cable numbers.
     * @return Connection ID.
     */
    int getConnectionID(const std::vector<int>& sectorCable) const;

    /**
     * Add new entry to the mapping table.
     * @param[in] connectionID Connection ID.
     * @param[in] sectorCable  Sector and cable number.
     */
    void addMapping(int connectionID, std::vector<int> sectorCable);

    /**
     * Print mapping parameters
     */
    void print() const;

  private:

    std::map<int, std::vector<int>> m_connection2cable; /**< map of crates to cables */

    ClassDef(ARICHBiasCrateCableMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
