/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Channel threshold value for all 512 channels of 16 modules.
   * From lase scans.
   */
  class TOPCalChannelThreshold: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPCalChannelThreshold()
    {
      for (unsigned m = 0; m < c_numModules; m++) {
        for (unsigned c = 0; c < c_numChannels; c++) {
          m_Thr[m][c] = 0;
        }
      }
    }

    /**
     * Sets thresholds for a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @param Thr channel threshold
     */
    void setThr(int moduleID, unsigned channel, double Thr)
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return;
      if (channel >= c_numChannels) return;
      m_Thr[module][channel] = Thr;
    }

    /**
     * Returns threshold of a single channel
     * @param moduleID module ID (1-based)
     * @param channel hardware channel number (0-based)
     * @return Thr
     */
    double getThr(int moduleID, unsigned channel) const
    {
      unsigned module = moduleID - 1;
      if (module >= c_numModules) return 0;
      if (channel >= c_numChannels) return 0;
      return m_Thr[module][channel];
    }


  private:

    /**
     * Sizes
     */
    enum {c_numModules = 16,  /**< number of modules */
          c_numChannels = 512 /**< number of channels per module */
         };


    float m_Thr[c_numModules][c_numChannels];    /**< threshold value */

    ClassDef(TOPCalChannelThreshold, 1); /**< ClassDef */

  };

} // end namespace Belle2

