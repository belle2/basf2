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

#define N_BIASCHANNELS 48

namespace Belle2 {

  /**
   * This class provides ARICH mapping of HAPD modules to bias
   * power supply channels.
   */
  class ARICHBiasChannelsMapping : public TObject {

  public:

    /**
     * Default constructor.
     */
    ARICHBiasChannelsMapping();

    /**
     * Get bias power supply channel ID from inner cable type and ID.
     * @param[in] crate        Crate ID number.
     * @param[in] slot         Slot ID number.
     * @param[in] connectionID Connection number.
     * @param[in] innerID      ID of inner cable.
     * @param[in] type         Type of inner cable.
     * @return Power supply channel ID number.
     */
    int getChannelID(int crate, int slot, int connectionID, int innerID, const std::string& type) const;

    /**
     * Get bias power supply cratem slot and channel ID from inner cable
     * type and ID.
     * @param[in] innerID      ID of inner cable.
     * @param[in] type         Type of inner cable.
     * @param[in] connectionID Connection number.
     * @return Power supply channel numbers; crate, slot, channelID.
     */
    std::vector<int> getChannelValues(int connectionID, int innerID, const std::string& type) const;

    /**
     * Get inner cable pin ID from bias power supply channel ID.
     * @param[in] channelID Power supply channel ID number.
     * @return Pin ID of inner cable.
     */
    int getPinID(std::vector<int> channelID) const;

    /**
     * Get bias/guard type from channel.
     * @param[in] channel Power supply channel numbers; crate, slot, channelID.
     * @return Inner cable type.
     */
    std::string getType(std::vector<int> channel) const;

    /**
     * Get inner cable pin ID from bias power supply channel ID.
     * @param[in] channel Power supply channel numbers; crate, slot, channelID.
     * @return ID of inner cable.
     */
    int getInnerID(std::vector<int> channel) const;

    /**
     * Get inner cable pin ID from bias power supply channel ID.
     * @param[in] channel Power supply channel numbers; crate, slot, channelID.
     * @return Connection number.
     */
    int getConnectionID(std::vector<int> channel) const;

    /**
     * Get inner cable ID and type, connected to bias channel.
     *
     * @param[in] channel
     * Vector of crate, slot and power supply channel ID number.
     *
     * @return Inner cable ID and type.
     */
    std::tuple<int, int, std::string> getInnerConnection(std::vector<int> channel) const;

    /**
     * Add new entry to the mapping table.
     * @param[in] crate        Crate ID number.
     * @param[in] slot         Slot ID number.
     * @param[in] channelID    Power supply channel ID number.
     * @param[in] pinID        Pin ID of inner cable.
     * @param[in] connectionID Connection number.
     * @param[in] innerID      ID of inner cable.
     * @param[in] type         Type of inner cable.
     */
    void addMapping(int crate, int slot, int channelID, int pinID, int connectionID, int innerID, const std::string& type);

    /**
     * Print properties
     */
    void print() const;

  private:

    std::map<std::vector<int>, std::tuple<int, int, int, std::string>>
        m_channel2type;   /**< power suply channel ID link to inner cable */

    ClassDef(ARICHBiasChannelsMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
