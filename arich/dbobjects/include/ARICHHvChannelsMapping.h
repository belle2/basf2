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

#define N_HVCHANNELS 24

namespace Belle2 {
  //! The Class for ARICH mapping of bias power supply channels to modules
  /*!     This class provides ARICH mapping of HAPD modules to bias power supply channels.
   */

  class ARICHHvChannelsMapping : public TObject {

  public:

    //! Default constructor
    ARICHHvChannelsMapping();

    /**
     * Get bias power supply channel ID from inner cable type and ID
     * @param connectionID ID of HV cable connection
     * @return pinID pin ID of inner cable
     * @return channel values; crate, slot, channel ID
     */
    std::vector<int> getChannelValues(int connectionID, int pinID) const;

    /**
    * Get inner cable pin ID from hv power supply channel ID
    * @param channel values; crate, slot, channel ID
    * @return pinID pin ID of inner cable
    */
    int getPinID(std::vector<int> channel) const;

    /**
    * Get connection ID from hv power supply channel ID
    * @param channel values; crate, slot, channel ID
    * @return connectionID ID of HV cable connection
    */
    int getConnectionID(std::vector<int> channel) const;

    /**
    * Get connection ID from hv power supply channel ID
    * @param connectionID ID of HV cable connection
    * @param pinID pin ID of inner cable
    * @return crate crate ID number
    */
    int getCrate(int connectionID, int pinID) const;

    /**
    * Get connection ID from hv power supply channel ID
    * @param connectionID ID of HV cable connection
    * @param pinID pin ID of inner cable
    * @return slot slot ID number
    */
    int getSlot(int connectionID, int pinID) const;

    /**
    * Get connection ID from hv power supply channel ID
    * @param connectionID ID of HV cable connection
    * @param pinID pin ID of inner cable
    * @return channelID channel ID number
    */
    int getChannel(int connectionID, int pinID) const;

    /**
     * Add new entry to the mapping table
     * @param channelID power supply channel ID number
     * @param pinID pin ID of inner cable
     * @param connectionID ID of HV cable connection
     */
    void addMapping(int crate, int slot, int channelID, int connectionID, int pinID);

  private:

    std::map<std::vector<int>, std::vector<int>> m_channel2type;   /**< power suply channel ID link to inner cable */

    ClassDef(ARICHHvChannelsMapping, 1);  /**< ClassDef, must be the last term before the closing {}*/

  };

} // namespace
