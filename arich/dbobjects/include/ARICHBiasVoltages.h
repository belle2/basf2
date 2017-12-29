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

    void print();

  private:

    std::map<std::string, std::vector<int>> m_hapd2voltages; /**< map of modules to cables */

    ClassDef(ARICHBiasVoltages, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
