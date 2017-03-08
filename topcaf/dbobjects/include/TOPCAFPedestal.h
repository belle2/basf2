/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Xiaolong Wang                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>
#include <TProfile.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <topcaf/dataobjects/EventHeaderPacket.h>

namespace Belle2 {

  /**
   * Calibration constants: pedestals
   */
  class TOPCAFPedestal: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPCAFPedestal()
    {}

    /**
     * Full constructor
     * @histogram profile
     */
    TOPCAFPedestal(topcaf_channel_id_t channel_ID, TProfile* profile)
    {
      m_channel = channel_ID;
      m_pedestal = profile;
    }

    /**
     * Profile histogram getter
     * @return m_profile
     */
    TProfile* getPedestal()  {return m_pedestal;}

    topcaf_channel_id_t getChannel() {return m_channel;}

  private:
    topcaf_channel_id_t m_channel = 0;
    TProfile* m_pedestal;

    ClassDef(TOPCAFPedestal, 1); /**< ClassDef */

  };

} // end namespace Belle2

