/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

