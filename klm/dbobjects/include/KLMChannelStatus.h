/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMElementNumberDefinitions.h>

/* ROOT headers. */
#include <TObject.h>

/* C++ headers. */
#include <cstdint>
#include <map>

namespace Belle2 {

  /**
   * KLM channel status.
   */
  class KLMChannelStatus : public TObject {

  public:

    /**
     * Channel status.
     */
    enum ChannelStatus {

      /** Unknown status (no data). */
      c_Unknown,

      /** Normally operating channel. */
      c_Normal,

      /** Dead channel (no signal). */
      c_Dead,

      /** Hot channel (large background). */
      c_Hot,

      /** Undetermined (used during calibration). */
      c_Undetermined,

    };

    /**
     * Constructor.
     */
    KLMChannelStatus();

    /**
     * Destructor.
     */
    ~KLMChannelStatus();

    /**
     * Get channel status.
     * @param[in] channel Channel number.
     */
    enum ChannelStatus getChannelStatus(KLMChannelNumber channel) const;

    /**
     * Set channel status.
     * @param[in] channel Channel number.
     * @param[in] status  Status.
     */
    void setChannelStatus(KLMChannelNumber channel, enum ChannelStatus status);

    /**
     * Set staus for all channels.
     * @param[in] status Status.
     */
    void setStatusAllChannels(enum ChannelStatus status);

    /**
     * Get number of active strips in the specified KLM module.
     * @param[in] module Module number.
     */
    int getActiveStripsInModule(KLMChannelNumber module) const;

    /**
     * Operator ==.
     */
    bool operator==(KLMChannelStatus& status);

    /**
     * Number of new channels with status c_Normal
     * that have a different status in another channel-status data.
     */
    unsigned int newNormalChannels(KLMChannelStatus& status);

  private:

    /** Channel data. */
    std::map<KLMChannelNumber, enum ChannelStatus> m_ChannelStatus;

    /** Class version. */
    ClassDef(Belle2::KLMChannelStatus, 1);

  };

}
