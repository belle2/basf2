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
  //! The Class for ARICH bias voltages at gain 40
  /*!     This class provides ARICH bias voltages at gain 40
   */

  class ARICHBiasVoltages : public TObject {

  public:

    //! Default constructor
    ARICHBiasVoltages();

    /**
    * Get inner cable ID and type, connected to bias channel
     * @param hapdID serial number of HAPD
     * @return biasVoltages vector of applied bias voltages
    */
    std::vector<int> getBiasVoltages(const std::string& hapdID) const;

    /**
     * Add new entry to the mapping table
     * @param hapdID serial number of HAPD
     * @param biasVoltages vector of applied bias voltages
     */
    void addVoltages(const std::string& hapdID, std::vector<int> biasVoltages);

    /**
    * Print voltages for PS channels
    */
    void print() const;

  private:

    std::map<std::string, std::vector<int>> m_hapd2voltages; /**< map of voltages to power supply */

    ClassDef(ARICHBiasVoltages, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
