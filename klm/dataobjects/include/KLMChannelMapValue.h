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

/* KLM headers. */
#include <klm/dataobjects/KLMChannelIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TNamed.h>

/* C++ headers. */
#include <cstdint>
#include <map>

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
    const T getChannelData(KLMChannelNumber channel) const
    {
      typename std::map<KLMChannelNumber, T>::const_iterator it;
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
    void setChannelData(KLMChannelNumber channel, const T data)
    {
      typename std::map<KLMChannelNumber, T>::iterator it;
      it = m_ChannelData.find(channel);
      if (it == m_ChannelData.end()) {
        m_ChannelData.insert(std::pair<KLMChannelNumber, T>(channel, data));
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
      KLMChannelIndex klmChannels;
      for (KLMChannelIndex& klmChannel : klmChannels)
        setChannelData(klmChannel.getKLMChannelNumber(), data);
    }

  private:

    /** Channel data. */
    std::map<KLMChannelNumber, T> m_ChannelData;

    /** Class version. */
    ClassDef(Belle2::KLMChannelMapValue<T>, 1);

  };

}
