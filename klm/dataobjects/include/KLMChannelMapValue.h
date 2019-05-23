/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TNamed.h>

/* Belle2 headers. */
#include <framework/logging/Logger.h>
#include <klm/dataobjects/BKLMChannelIndex.h>
#include <klm/dataobjects/EKLMChannelIndex.h>

namespace Belle2 {

  /**
   * KLM channel map.
   */
  template<class T> class KLMChannelMapValue : public TNamed {

  public:

    /**
     * Constructor.
     */
    KLMChannelMapValue()
    {
    }

    /**
     * Destructor.
     */
    ~KLMChannelMapValue()
    {
    }

    /**
     * Get channel data.
     * @param[in] channel Channel number.
     */
    const T getChannelData(uint16_t channel) const
    {
      typename std::map<uint16_t, T>::const_iterator it;
      it = m_ChannelData.find(channel);
      if (it == m_ChannelData.end()) {
        B2ERROR("No data for KLM channel in map." <<
                LogVar("Channel number", channel));
      }
      return it->second;
    }

    /**
     * Set channel data.
     * @param[in] channel Channel number.
     * @param[in] data    Data.
     */
    void setChannelData(uint16_t channel, const T data)
    {
      typename std::map<uint16_t, T>::iterator it;
      it = m_ChannelData.find(channel);
      if (it == m_ChannelData.end()) {
        m_ChannelData.insert(std::pair<uint16_t, T>(channel, data));
      } else {
        it->second = data;
      }
    }

    /**
     * Set data for all channels.
     * @param[in] data Data.
     */
    void setDataAllChannels(const T data)
    {
      BKLMChannelIndex bklmChannels;
      for (BKLMChannelIndex& bklmChannel : bklmChannels)
        setChannelData(bklmChannel.getKLMChannelNumber(), data);
      EKLMChannelIndex eklmChannels;
      for (EKLMChannelIndex& eklmChannel : eklmChannels)
        setChannelData(eklmChannel.getKLMChannelNumber(), data);
    }

  private:

    /** Channel data. */
    std::map<uint16_t, T> m_ChannelData;

    /** Class version. */
    ClassDef(Belle2::KLMChannelMapValue<T>, 1);

  };

}
