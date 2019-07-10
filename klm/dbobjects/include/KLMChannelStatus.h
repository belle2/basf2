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
#include <TObject.h>

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

      /** Unknown status. */
      c_Unknown,

      /** Normally operating channel. */
      c_Normal,

      /** Dead channel (no signal). */
      c_Dead,

      /** Hot channel (large background). */
      c_Hot,

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
    enum ChannelStatus getChannelStatus(uint16_t channel) const;

    /**
     * Set channel status.
     * @param[in] channel Channel number.
     * @param[in] status  Status.
     */
    void setChannelStatus(uint16_t channel, enum ChannelStatus status);

    /**
     * Set staus for all channels.
     * @param[in] status Status.
     */
    void setStatusAllChannels(enum ChannelStatus status);

    /**
     * Get number of active strips in the specified EKLM sector.
     * @param[in] sectorGlobal Sector global number.
     */
    int getActiveStripsEKLMSector(int sectorGlobal) const;

  private:

    /** Channel data. */
    std::map<uint16_t, enum ChannelStatus> m_ChannelStatus;

    /** Class version. */
    ClassDef(Belle2::KLMChannelStatus, 1);

  };

}
