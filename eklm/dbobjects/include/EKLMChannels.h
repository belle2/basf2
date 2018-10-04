/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMCHANNELS_H
#define EKLMCHANNELS_H

/* C++ headers. */
#include <cstdint>
#include <map>

/* External headers. */
#include <TObject.h>

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMChannelData.h>

namespace Belle2 {

  /**
   * Class to store EKLM alignment data in the database.
   */
  class EKLMChannels : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMChannels();

    /**
     * Destructor.
     */
    ~EKLMChannels();

    /**
     * Set channel data.
     * @param[in] strip Strip (channel) number.
     * @param[in] dat   Channels data.
     */
    void setChannelData(uint16_t strip, EKLMChannelData* dat);

    /**
     * Get channel data.
     * @param[in] strip Strip (channel) number.
     */
    const EKLMChannelData* getChannelData(uint16_t strip) const;

  private:

    /** Channel data. */
    std::map<uint16_t, EKLMChannelData> m_Channels;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMChannels, 1);

  };

}

#endif

