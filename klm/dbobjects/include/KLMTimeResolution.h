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
   * Class to store KLM time resolution in the database.
   */
  class KLMTimeResolution : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMTimeResolution()
    {
    }

    /**
     * Destructor.
     */
    ~KLMTimeResolution()
    {
    }

    /**
     * Get time resolution.
     * @param[in] channel Channel number.
     */
    float getTimeResolution(KLMChannelNumber channel) const;

    /**
     * Set time resolution.
     * @param[in] channel    Channel number.
     * @param[in] resolution Resolution.
     */
    void setTimeResolution(KLMChannelNumber channel, float resolution);

  private:

    /** Time resolution. */
    std::map<KLMChannelNumber, float> m_TimeResolution;

    /** Class version. */
    ClassDef(Belle2::KLMTimeResolution, 1);

  };

}
