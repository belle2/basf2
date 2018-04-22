/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * Class to store Monte Carlo information useful for trigger studies
   */
  class TOPTriggerMCInfo: public TObject {
  public:

    /**
     * Default constructor
     */
    TOPTriggerMCInfo()
    {}

    /**
     * setter for generated time stamp of interaction
     */
    void setBunchTimeStamp(int timeStamp) {m_bunchTimeStamp = timeStamp;}

    /**
     * getter for generated time stamp of interaction
     */
    int getBunchTimeStamp() const {return m_bunchTimeStamp;}

  private:

    int m_bunchTimeStamp = 0; /**< generated time stamp of the interaction */

    ClassDef(TOPTriggerMCInfo, 1); /**< ClassDef */


  };

}

