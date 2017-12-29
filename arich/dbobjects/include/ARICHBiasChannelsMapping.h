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

#define N_BIASCHANNELS 48

namespace Belle2 {
  //! The Class for ARICH mapping of bias power supply channels to modules
  /*!     This class provides ARICH mapping of HAPD modules to bias power supply channels.
   */

  class ARICHBiasChannelsMapping : public TObject {

  public:

    //! Default constructor
    ARICHBiasChannelsMapping();

    /**
     * Get bias power supply channel ID from inner cable type and ID
     * @param crate crate ID number
     * @param slot slot ID number
     * @param channelID power supply channel ID number
     * @param innerID ID of inner cable
     * @param type type of inner cable
     * @return channelID power supply channel ID number
     */
    int getChannelID(int crate, int slot, int connectionID, int innerID, std::string type) const;

    /**
     * Get bias power supply cratem slot and channel ID from inner cable type and ID
     * @param innerID ID of inner cable
     * @param type type of inner cable
     * @param connectionID connection number
     * @return channel power supply channel numbers; crate, slot, channelID
     */
    std::vector<int> getChannelValues(int connectionID, int innerID, std::string type) const;

    /**
    * Get inner cable pin ID from bias power supply channel ID
    * @param channelID power supply channel ID number
    * @return pinID pin ID of inner cable
    */
    int getPinID(std::vector<int> channelID) const;

    /**
    * Get bias/guard type from channel
    * @param channel power supply channel numbers; crate, slot, channelID
    * @param type type of inner cable
    */
    std::string getType(std::vector<int> channel) const;

    /**
    * Get inner cable pin ID from bias power supply channel ID
    * @param channel power supply channel numbers; crate, slot, channelID
    * @return innerID ID of inner cable
    */
    int getInnerID(std::vector<int> channel) const;

    /**
    * Get inner cable pin ID from bias power supply channel ID
    * @param channel power supply channel numbers; crate, slot, channelID
    * @return connectionID connection number
    */
    int getConnectionID(std::vector<int> channel) const;

    /**
    * Get inner cable ID and type, connected to bias channel
    * @param channel vector of crate, slot and power supply channel ID number
    * @return innerID inner cable ID and type
    */
    std::tuple<int, int, std::string> getInnerConnection(std::vector<int> channel) const;

    /**
     * Add new entry to the mapping table
     * @param crate crate ID number
     * @param slot slot ID number
     * @param channelID power supply channel ID number
     * @param pinID pin ID of inner cable
     * @param connectionID connection number
     * @param innerID ID of inner cable
     * @param type type of inner cable
     */
    void addMapping(int crate, int slot, int channelID, int pinID, int connectionID, int innerID, const std::string& type);

    /**
     * Print properties
     */
    void print();

  private:

    std::map<std::vector<int>, std::tuple<int, int, int, std::string>>
        m_channel2type;   /**< power suply channel ID link to inner cable */

    ClassDef(ARICHBiasChannelsMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
