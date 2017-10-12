/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMCHANNELDATA_H
#define EKLMCHANNELDATA_H

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM channel data.
   */
  class EKLMChannelData : public TObject {

  public:

    /**
     * Constructor.
     */
    EKLMChannelData();

    /**
     * Destructor.
     */
    ~EKLMChannelData();

    /**
     * Set if channel is active or not.
     */
    void setActive(bool active);

    /**
     * Get if channel is active or not.
     */
    bool getActive() const;

  private:

    /** True if channel is active. */
    bool m_Active;

    /** Makes objects storable. */
    ClassDef(Belle2::EKLMChannelData, 1);

  };

}

#endif

