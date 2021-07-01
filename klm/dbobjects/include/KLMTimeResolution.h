/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

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
    float getTimeResolution(uint16_t channel) const;

    /**
     * Set time resolution.
     * @param[in] channel    Channel number.
     * @param[in] resolution Resolution.
     */
    void setTimeResolution(uint16_t channel, float resolution);

  private:

    /** Time resolution. */
    std::map<uint16_t, float> m_TimeResolution ;

    /** Class version. */
    ClassDef(Belle2::KLMTimeResolution, 1);

  };

}
